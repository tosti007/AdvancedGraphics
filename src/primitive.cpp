#include <math.h>       /* sqrt */

#include "precomp.h" // include (only) this in every .cpp file
#include "primitive.h"

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
