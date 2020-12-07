#pragma once

#include "vectors.h"
#include "ray.h"
#include "primitive.h"
#include "vectors.h"

namespace AdvancedGraphics
{

struct BVH; // forward declaration

struct BVHNode
{
  public:
	aabb bounds;
	size_t firstleft; 
	uint count; // number of triangles
	Triangle *Traverse( BVH *bvh, Ray *r, int depth );
	void Subdivide( BVH *bvh );
	bool AABBIntersection( const Ray *r, const aabb &bb, float &tmin, float &tmax );
};

struct BVH
{
  public:
	BVHNode *pool;
	uint nr_nodes, nr_nodes_max;

	BVHNode *root;
	Triangle *triangles;
	uint *indices;

	inline Triangle* Traverse( Ray *r ) { return root->Traverse(this, r, 0); }
	void ConstructBVH( Triangle *triangles, uint triangleCount );
	static aabb ComputeBounds( const Triangle *triangles, int firstleft, uint count );
};

}; // namespace AdvancedGraphics