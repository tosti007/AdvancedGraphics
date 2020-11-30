#include <float.h>

#include "precomp.h" // include (only) this in every .cpp file
#include "light.h"
#include "utils.h"
#include "primitive.h"

PointLight::PointLight( vec3 pos, Color col ) : 
	Light(col),
	position( pos )
{
}

SphereLight::SphereLight( vec3 p, float r, Color c ) :
	Light(c),
	position(p),
	radius(r)
{
}

bool SphereLight::Intersect(Ray* r)
{
	vec3 C = position - r->origin;
    float t = dot(C, r->direction);
    vec3 Q = C - t * r->direction;
    float p2 = dot(Q, Q);
    float r2 = radius * radius;
    if (p2 > r2) return false;
    t -= sqrt(r2 - p2);
    if (t <= 0 || t >= r->t) return false;
    r->t = t;
    return true;
}

vec3 SphereLight::PointOnLight()
{
	return RandomPointOnSphere(radius);
}