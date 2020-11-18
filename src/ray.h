#pragma once

#include "vectors.h"
#include "color.h"

class Primitive; // Forward declaration

namespace AdvancedGraphics {

struct Ray
{
	vec3 origin, direction;
    float t;
    Primitive* obj;

    Ray( vec3 o, vec3 d );
};

}; // namespace AdvancedGraphics