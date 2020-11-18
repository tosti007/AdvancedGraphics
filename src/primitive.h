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

class Sphere : Primitive
{
    public:
    vec3 position;
    float radius;

    Sphere( vec3 p, float r, Color c );
    bool Intersect(Ray* r);
};
