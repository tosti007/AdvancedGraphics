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
	bool Traverse( BVH *bvh, Ray *r, uint &depth );
	void Subdivide( BVH *bvh );
	void RecomputeBounds( const BVH *bvh );
	bool AABBIntersection( const Ray *r, const aabb &bb, float &tmin, float &tmax );
	void Print(BVH* bvh, uint depth);
  private:
	void Subdivide_Binned_Simple( BVH* bvh );
	void Subdivide_Binned( BVH *bvh );
	void Subdivide_Median( BVH *bvh );
	void Subdivide_SAH( BVH *bvh );
	bool SAH( BVH *bvh, int &bestAxis, float &bestSplitLocation );
	void Divide( BVH *bvh, int &bestAxis, float &bestSplitLocation );
};

struct BVH
{
  public:
	BVHNode *pool;
	uint nr_nodes, nr_nodes_max;

	BVHNode *root;
	Triangle *triangles;
	uint nr_triangles;
	uint *indices;

	inline bool Traverse( Ray *r, uint &depth ) { if (nr_triangles > 0) return root->Traverse(this, r, ++depth); return false; }
	void ConstructBVH( Triangle *triangles, uint triangleCount );
	void Print();
};

}; // namespace AdvancedGraphics