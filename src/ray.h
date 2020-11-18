#pragma once

#include "vectors.h"
#include "color.h"

namespace AdvancedGraphics {

struct Ray
{
	vec3 origin, direction;
    Color color;
    float t;

    Ray( vec3 o, vec3 d );
};

}; // namespace AdvancedGraphics