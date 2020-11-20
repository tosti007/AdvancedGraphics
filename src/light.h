#pragma once

#include "color.h"
#include "vectors.h"

namespace AdvancedGraphics
{

struct Light
{
	vec3 position;
	Color color;

	Light( vec3 pos, Color col );
};

}; // namespace AdvancedGraphics