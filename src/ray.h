#pragma once

#include "vectors.h"
#include "color.h"

class Primitive; // Forward declaration

namespace AdvancedGraphics {

struct Ray
{
	vec3 origin, direction;
    float t;
    Primitive *obj;

    Ray( vec3 o, vec3 d );

    void Reflect(vec3 i, vec3 n);
    void Reflect(vec3 i, vec3 n, float angle);
    inline void Offset(float epsilon) { origin += CalculateOffset(epsilon); }

    vec3 CalculateOffset(float epsilon);
};

}; // namespace AdvancedGraphics