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
	// From: https://mathworld.wolfram.com/SpherePointPicking.html
	// Equation 12, 13, and 14
	float x0, x1, x2, x3;
	float x02, x12, x22, x32;
	float sum;
	do {
		x0 = RandomFloat() * 2 - 1;
		x1 = RandomFloat() * 2 - 1;
		x2 = RandomFloat() * 2 - 1;
		x3 = RandomFloat() * 2 - 1;
		x02 = x0 * x0;
		x12 = x1 * x1;
		x22 = x2 * x2;
		x32 = x3 * x3;
		sum = x02 + x12 + x22 + x32;
	} while( sum <= 1);
	return (radius / sum) * vec3(
		2 * (x1 * x3 + x0 * x2), 
		2 * (x2 * x3 + x0 * x1),
		x02 + x32 - x12 -x22
		);
}