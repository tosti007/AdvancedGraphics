#include <float.h>

#include "precomp.h" // include (only) this in every .cpp file
#include "light.h"
#include "utils.h"
#include "primitive.h"

SphereLight::SphereLight( vec3 p, float r, Color c ) :
	Light(c),
	position(p),
	radius(r)
{
}

bool SphereLight::Intersect(Ray* r)
{
	// Compute intersection point
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

bool SphereLight::Occludes( Ray *r )
{
	vec3 C = position - r->origin;
	float t = dot( C, r->direction );
	vec3 Q = C - t * r->direction;
	float p2 = dot( Q, Q );
	float r2 = radius * radius;
	if ( p2 > r2 ) return -1;
	t -= sqrt( r2 - p2 );

	if ( t <= 0 || t >= r->t ) return false;

	return true;
}

vec3 SphereLight::PointOnLight()
{
	return RandomPointOnSphere(radius) + position;
}

vec3 SphereLight::NormalAt( vec3 point )
{
	return ( 1 / radius ) * ( point - position );
}

float SphereLight::Area()
{
	return PI * radius * radius;
}