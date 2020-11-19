#pragma once

#include "vectors.h"
#include "ray.h"
#include "color.h"
#include "tiny_obj_loader.h"

class Primitive
{
    public:
    Color color;

    inline Primitive() = default;
    inline Primitive( Pixel c) : color(PixelToColor(c)) {}
    inline Primitive( Color c) : color(c) {}
    virtual bool Intersect(Ray* r) = 0;
};

class Plane : public Primitive
{ // Normally a plane has a d value, but let's just assume that's always 0
    public:
    vec3 normal;
    float dist;

    Plane(vec3 n, float d, Pixel c);
    bool Intersect(Ray* r);
};

class Sphere : public Primitive
{
    public:
    vec3 position;
    float radius;

    Sphere( vec3 p, float r, Pixel c );
    bool Intersect(Ray* r);
};

class Triangle : public Primitive
{
    public:
    vec3 p0, p1, p2;
    vec3 normal;

    Triangle() = default;;
    Triangle( vec3 v0, vec3 v1, vec3 v2, Pixel c);
    Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c);
    bool Intersect(Ray* r);

    static vec3 ComputeNormal( vec3 v0, vec3 v1, vec3 v2 );
    static Triangle FromTinyObj( tinyobj::attrib_t* attrib, tinyobj::mesh_t* mesh, size_t f );
};

class TriangleSoup : public Primitive
{
    // I just realised that a color value is really weird on a triangle soup with each their own color.
    public:
    uint nr_faces;
    Triangle* faces;

    TriangleSoup(Triangle* fs, uint nfs, Pixel c);
    bool Intersect(Ray* r);

    static void FromTinyObj( TriangleSoup* soup, tinyobj::attrib_t* attrib, tinyobj::mesh_t* mesh);
};
