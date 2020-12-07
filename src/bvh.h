#pragma once

#include "vectors.h"
#include "ray.h"
#include "primitive.h"
#include "vectors.h"

namespace AdvancedGraphics
{

struct BVH; // forward declaration

struct Intersection
{
  public:
	Triangle *triangle;
	vec3 location;
	float distance = INFINITY;
};

struct BVHNode
{
  public:
	aabb bounds;
	size_t firstleft; 
	uint count; // number of triangles
	void Traverse( Ray r, BVH *bvh, Intersection& interPoint, int &depth );
	void Subdivide( BVHNode *pool, uint *indices, const Triangle *triangles );
	bool AABBIntersection( const Ray &r, const aabb &bb, float &tmin, float &tmax );
};

struct BVH
{
  public:
	BVHNode *pool;
	BVHNode *root;
	Triangle *triangles;
	uint *indices;
	void ConstructBVH( Triangle *triangles, uint triangleCount );
	static aabb ComputeBounds( const Triangle *triangles, int firstleft, uint count );
};

}; // namespace AdvancedGraphics