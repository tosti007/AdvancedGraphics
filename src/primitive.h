#pragma once

#include "vectors.h"
#include "ray.h"
#include "color.h"
#include "tiny_obj_loader.h"

struct Material
{
	float reflective;
	float speculative;
};

class Primitive
{
    public:
    Color color;
    Material material;

    inline Primitive() = default;
    inline Primitive( Pixel c) : color(PixelToColor(c)) {}
    inline Primitive( Color c) : color(c) {}
    virtual bool Intersect(Ray* r) = 0;
	virtual vec3 NormalAt( vec3 point ) = 0;
};

class Plane : public Primitive
{ // Normally a plane has a d value, but let's just assume that's always 0
    public:
    vec3 normal;
    float dist;
	Material material;

    Plane( vec3 n, float d, Pixel c );
	Plane( vec3 n, float d, Pixel c, Material m );
    bool Intersect(Ray* r);
	vec3 NormalAt( vec3 point );
};

class Sphere : public Primitive
{
    public:
    vec3 position;
    float radius;
    Material material;

	Sphere( vec3 p, float r, Pixel c );
	Sphere( vec3 p, float r, Pixel c, Material m );
    bool Intersect(Ray* r);
    vec3 NormalAt( vec3 point );
};

class Triangle : public Primitive
{
    public:
    vec3 p0, p1, p2;
    vec3 normal;
	Material material;

    Triangle() = default;
    Triangle( vec3 v0, vec3 v1, vec3 v2, Pixel c);
	Triangle( vec3 v0, vec3 v1, vec3 v2, Pixel c, Material m );
	Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c );
	Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c, Material m);
    bool Intersect(Ray* r);

	vec3 NormalAt( vec3 point );
    static vec3 ComputeNormal( vec3 v0, vec3 v1, vec3 v2 );
    static void FromTinyObj( Triangle* tri, tinyobj::attrib_t* attrib, tinyobj::mesh_t* mesh, size_t f );
};
