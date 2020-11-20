#include <float.h>

#include "precomp.h" // include (only) this in every .cpp file
#include "light.h"

Light::Light( vec3 pos, Color col ) : 
	position( pos ),
	color( col )
{
}
