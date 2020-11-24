#pragma once

#include "vectors.h"
#include "color.h"

class Primitive; // Forward declaration

namespace AdvancedGraphics {

struct Ray
{
	vec3 origin, direction;
    float t;

    Ray( vec3 o, vec3 d );

    void Reflect(vec3 i, vec3 n);
};

}; // namespace AdvancedGraphics