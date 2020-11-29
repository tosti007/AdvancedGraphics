#pragma once

#include "color.h"
#include "vectors.h"

namespace AdvancedGraphics
{

struct Light
{
	Color color;

	inline Light(Color c) : color(c) {}

	virtual vec3 PointOnLight() = 0;
};

struct PointLight : Light
{
	vec3 position;

	PointLight( vec3 pos, Color col );

	inline vec3 PointOnLight() { return position; }
};

}; // namespace AdvancedGraphics