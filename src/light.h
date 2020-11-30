#pragma once

#include "color.h"
#include "vectors.h"
#include "ray.h"

namespace AdvancedGraphics
{

struct Light
{
	Color color;

	inline Light(Color c) : color(c) {}

    virtual bool Intersect(Ray* r) = 0;
	virtual vec3 PointOnLight() = 0;
};

struct PointLight : Light
{
	vec3 position;

	PointLight( vec3 pos, Color col );

    inline bool Intersect(Ray* r) { return false; }
	inline vec3 PointOnLight() { return position; }
};

struct SphereLight : Light 
{
	vec3 position;
	float radius;

	SphereLight( vec3 p, float r, Color c );

    bool Intersect(Ray* r);
	vec3 PointOnLight();
};

}; // namespace AdvancedGraphics