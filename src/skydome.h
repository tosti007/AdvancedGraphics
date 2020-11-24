#pragma once

#include "vectors.h"
#include "color.h"

#define SKYDOME_DEFAULT_COLOR Color(0, 0, 0)

struct SkyDome
{
	uint width, height;
    Color* pixels;

    SkyDome();
    Color FindColor(vec3 direction);
};
