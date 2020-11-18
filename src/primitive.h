#pragma once

#include "vectors.h"
#include "ray.h"
#include "color.h"

// TODO: Add some abstract class

class Sphere
{
    public:
    vec3 position;
    float radius;
    Color color;

    Sphere( vec3 p, float r, Color c );
    bool Intersect(Ray* r);
};
