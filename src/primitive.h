#pragma once

#include <map>

#include "vectors.h"
#include "ray.h"
#include "color.h"
#include "tiny_obj_loader.h"
#include "utils.h"
#include "surface.h"

struct Material
{
public:
    inline Material() : Material(0, 0, 1, DEFAULT_OBJECT_COLOR, nullptr) {}
    inline Material(float flect, float fract, float ir, Color c, Surface* tex) :
        color(c),
        texture(tex),
        reflection(std::max(0.0f, flect)),
        refraction(std::max(0.0f, fract)),
        ior(std::max(0.0f, ir))
        {
            assert(flect + fract <= 1.0f);
        }

    // TODO: Replace this with an epsilon?
    inline bool HasReflection() { return reflection > 0.0f; }
    inline bool HasRefraction() { return refraction > 0.0f; }

    inline float GetDiffuse() { return 1 - reflection - refraction; }
    inline float GetReflection() { return reflection; }
    inline float GetRefraction() { return refraction; }
    inline float GetIoR() { return ior; }
    
    inline Color InternalColor() { return color; }
    inline Color TextureAt ( vec2 point ) {
        if (texture == nullptr) {
            return color;
        }
        int idx = point.x + point.y * texture->GetWidth();
        if (idx < 0 || idx >= texture->GetWidth() * texture->GetHeight())
            return color;
        return texture->GetBuffer()[idx];
    }

	static void FromTinyObj( Material *res, std::string basedir, tinyobj::material_t mat );
private:
    Color color;
    Surface* texture;
    // lambert bsdf properties
	// Data for a basic Lambertian BRDF, augmented with pure specular reflection and
	// refraction. Assumptions:
	// diffuse component = 1 - (reflection + refraction); 
	// (reflection + refraction) < 1;
	// ior is the index of refraction of the medium below the shading point.
	float reflection, refraction, ior;
};

class Primitive
{
    public:
    int material;

    inline Primitive() : material(-1) {}
    inline Primitive(int m) : material(m) {}

    // If a negative value is returned, no intersection is found.
    virtual float IntersectionDistance(Ray* r) = 0;
    // This sets r.t if an intersetion is found.
    bool Intersect(Ray* r);
    bool Occludes(Ray* r);
    // This must return a normalized vector
	virtual vec3 NormalAt( vec3 point ) = 0;
    inline Color ColorAt( Material* materials, vec3 point )
    {
        if (material < 0)
            return DEFAULT_OBJECT_COLOR;
        return materials[material].TextureAt(TextureAt(point));
    }
    virtual vec2 TextureAt( vec3 point ) = 0;
};

class Sphere : public Primitive
{
    public:
    vec3 position;
    float radius;

    Sphere() = default;
	inline Sphere( vec3 p, float r) : Sphere(p, r, -1) {}
	Sphere( vec3 p, float r, int m );

    // Inherited functions
    float IntersectionDistance(Ray* r);
    vec3 NormalAt( vec3 point );
    vec2 TextureAt ( vec3 point );
};

class Triangle : public Primitive
{
    public:
    vec3 p0, p1, p2;
    vec3 normal;
    vec2 t0, t1, t2;

    Triangle() = default;
    inline Triangle( vec3 v0, vec3 v1, vec3 v2) : Triangle( v0, v1, v2, -1) {}
	inline Triangle( vec3 v0, vec3 v1, vec3 v2, int m ) : Triangle(v0, v1, v2, ComputeNormal(v0, v1, v2), m) {}
	inline Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n ) : Triangle( v0, v1, v2, n, -1 ) {}
	Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, int m);

    // Inherited functions
    float IntersectionDistance(Ray* r);
	vec3 NormalAt( vec3 point );
    vec2 TextureAt ( vec3 point );

    // Static functions
    static vec3 ComputeNormal( vec3 v0, vec3 v1, vec3 v2 );
	static void FromTinyObj( Triangle *tri, tinyobj::attrib_t *attrib, tinyobj::mesh_t *mesh, size_t f, std::vector<tinyobj::material_t> materials );
};
