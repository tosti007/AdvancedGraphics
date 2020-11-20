#pragma once

#include "vectors.h"
#include "ray.h"
#include "color.h"
#include "tiny_obj_loader.h"

struct Material
{
	float reflective;
	float speculative;
    float diffuse;

    inline Material(float r, float s, float d) : reflective(r), speculative(s), diffuse(d) {}
};

class Primitive
{
    public:
    Color color;
    Material* material;

    inline Primitive() = default;
    inline Primitive( Pixel c, Material* m) : Primitive(PixelToColor(c), m) {}
    inline Primitive( Color c, Material* m) : color(c), material(m) {}
    virtual bool Intersect(Ray* r) = 0;
	virtual vec3 NormalAt( vec3 point ) = 0;
};

class Plane : public Primitive
{ // Normally a plane has a d value, but let's just assume that's always 0
    public:
    vec3 normal;
    float dist;

    inline Plane( vec3 n, float d, Pixel c ) : Plane(n, d, c, NULL) {}
	Plane( vec3 n, float d, Pixel c, Material* m );
    bool Intersect(Ray* r);
	vec3 NormalAt( vec3 point );
};

class Sphere : public Primitive
{
    public:
    vec3 position;
    float radius;

	inline Sphere( vec3 p, float r, Pixel c ) : Sphere(p, r, c, NULL) {}
	Sphere( vec3 p, float r, Pixel c, Material* m );
    bool Intersect(Ray* r);
    vec3 NormalAt( vec3 point );
};

class Triangle : public Primitive
{
    public:
    vec3 p0, p1, p2;
    vec3 normal;

    Triangle() = default;
    inline Triangle( vec3 v0, vec3 v1, vec3 v2, Pixel c) : Triangle( v0, v1, v2, c, NULL) {}
	Triangle( vec3 v0, vec3 v1, vec3 v2, Pixel c, Material* m );
	inline Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c ) : Triangle( v0, v1, v2, n, c, NULL ) {}
	Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c, Material* m);
    bool Intersect(Ray* r);

	vec3 NormalAt( vec3 point );
    static vec3 ComputeNormal( vec3 v0, vec3 v1, vec3 v2 );
    static void FromTinyObj( Triangle* tri, tinyobj::attrib_t* attrib, tinyobj::mesh_t* mesh, size_t f );
};
