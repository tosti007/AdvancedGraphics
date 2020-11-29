#pragma once

#include "color.h"
#include "vectors.h"

namespace AdvancedGraphics
{

struct PointLight
{
	vec3 position;
	Color color;

	PointLight( vec3 pos, Color col );
};

}; // namespace AdvancedGraphics