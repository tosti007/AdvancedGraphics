#include <math.h>       /* sqrt */

#include "precomp.h" // include (only) this in every .cpp file
#include "primitive.h"

Plane::Plane( vec3 n, float d, Color c ) :
    Primitive(c),
    normal(n.normalized()),
    dist(d)
{
}

bool Plane::Intersect(Ray* r) 
{
    float t = -(dot(r->origin, normal) + dist) / dot(r->direction, normal);

    if (t >= r->t || t <= 0) return false;
    r->t = t;
    r->color = color;
    return true;
}

Sphere::Sphere( vec3 p, float r, Color c ) :
    Primitive(c),
    position(p),
    radius(r)
{
}

bool Sphere::Intersect(Ray* r)
{
    vec3 C = position - r->origin;
    float t = dot(C, r->direction);
    vec3 Q = C - t * r->direction;
    float p2 = dot(Q, Q);
    float r2 = radius * radius;
    if (p2 > r2) return false;
    t -= sqrt(r2 - p2);
    
    if (t >= r->t || t <= 0) return false;
    r->t = t;
    r->color = color;
    return true;
}
