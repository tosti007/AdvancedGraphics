#pragma once

#include "vectors.h"
#include "ray.h"
#include "color.h"

// TODO: Add some abstract class

class Primitive
{
    public:
    Color color;

    inline Primitive( Color c) : color(c) {}
    virtual bool Intersect(Ray* r) = 0;
};

class Plane : Primitive
{ // Normally a plane has a d value, but let's just assume that's always 0
    public:
    vec3 normal;
    float dist;

    Plane(vec3 n, float d, Color c);
    bool Intersect(Ray* r);
};

class Sphere : Primitive
{
    public:
    vec3 position;
    float radius;

    Sphere( vec3 p, float r, Color c );
    bool Intersect(Ray* r);
};

class Triangle : Primitive
{
    public:
    vec3 p0, p1, p2;

    Triangle( vec3 v0, vec3 v1, vec3 v2, Color c);
    bool Intersect(Ray* r);
};
