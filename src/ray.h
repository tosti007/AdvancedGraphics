#pragma once

#include "vectors.h"

namespace AdvancedGraphics {

struct Ray
{
	vec3 origin, direction;
    float t;

    Ray( vec3 o, vec3 d );
};

}; // namespace AdvancedGraphics