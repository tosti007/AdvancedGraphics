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
    float speculative, refractive, density;

    inline Material() : Material(0, 0, 0) {}   
    inline Material(float s, float r, float d) :
        speculative(clamp(s, 0.0f, 1.0f)),
        refractive(std::max(r, 0.0f)),
        density(clamp(d, 0.0f, 1.0f))
        {}

    inline bool HasReflect() { return speculative > 0.0f; }
    inline bool HasRefract() { return refractive > 0.0f; }

    inline bool IsFullMirror() { return speculative == 1.0f; }
    inline bool IsFullDiffuse() { return speculative == 0.0f; }
    inline bool IsNotRefractive() { return refractive == 0.0f; }
};

class Primitive
{
    public:
    Material* material;
    Surface* texture;

    inline Primitive() = default;
    inline Primitive( Color c, Material* m) : material(m), texture(nullptr), color(c) {}

    // If a negative value is returned, no intersection is found.
    virtual float IntersectionDistance(Ray* r) = 0;
    // This sets r.t if an intersetion is found.
    bool Intersect(Ray* r);
    bool Occludes(Ray* r);
    // This must return a normalized vector
	virtual vec3 NormalAt( vec3 point ) = 0;
    Color ColorAt( vec3 point );
    inline Color InternalColor() { return color; }
    
    protected:
    Color color;
    virtual int TextureAt ( vec3 point ) = 0;
};

class Sphere : public Primitive
{
    public:
    vec3 position;
    float radius;

    Sphere() = default;
	inline Sphere( vec3 p, float r, Color c ) : Sphere(p, r, c, NULL) {}
	Sphere( vec3 p, float r, Color c, Material* m );
    float IntersectionDistance(Ray* r);
    vec3 NormalAt( vec3 point );
    int TextureAt ( vec3 point );
};

class Triangle : public Primitive
{
    public:
    vec3 p0, p1, p2;
    vec3 normal;
    vec2 t0, t1, t2;

    Triangle() = default;
    inline Triangle( vec3 v0, vec3 v1, vec3 v2, Color c) : Triangle( v0, v1, v2, c, NULL) {}
	inline Triangle( vec3 v0, vec3 v1, vec3 v2, Color c, Material* m ) : Triangle(v0, v1, v2, ComputeNormal(v0, v1, v2), c, m) {}
	inline Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c ) : Triangle( v0, v1, v2, n, c, NULL ) {}
	Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c, Material* m);
    float IntersectionDistance(Ray* r);

	vec3 NormalAt( vec3 point );
    static vec3 ComputeNormal( vec3 v0, vec3 v1, vec3 v2 );
	static void FromTinyObj( Triangle *tri, tinyobj::attrib_t *attrib, tinyobj::mesh_t *mesh, size_t f, std::vector<tinyobj::material_t> materials, std::map<std::string, Surface*> textures );
    int TextureAt ( vec3 point );
};
