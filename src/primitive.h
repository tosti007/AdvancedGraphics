#pragma once

#include "vectors.h"
#include "ray.h"
#include "color.h"

// TODO: Add some abstract class

class Primitive
{
    public:
    Color color;

    inline Primitive( Pixel c) : color(PixelToColor(c)) {}
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

    Triangle( vec3 v0, vec3 v1, vec3 v2, Pixel c);
    bool Intersect(Ray* r);
};
