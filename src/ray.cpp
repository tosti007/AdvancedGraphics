#include <float.h>

#include "precomp.h" // include (only) this in every .cpp file
#include "ray.h"

 // We use the max float value for t, so intersections will be easier.
Ray::Ray( vec3 o, vec3 d ) :
    origin(o), 
    direction(d),
    t(INFINITY)
{
}

void Ray::Reflect(vec3 i, vec3 n)
{
    Reflect(i, n, dot(direction, n));
}

void Ray::Reflect(vec3 i, vec3 n, float angle)
{
	origin = i;
    direction -= 2 * angle * n;
	t = INFINITY;
}

vec3 Ray::CalculateOffset(float epsilon)
{
    // TODO: use some blend between direction and normal, depending on the fac value (dot of normal and direction)
    return epsilon * direction;
}