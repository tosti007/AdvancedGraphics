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
	bool Traverse( BVH *bvh, Ray *r, uint &depth, bool checkOcclusion );
	void Subdivide( BVH *bvh, aabb* triangle_bounds );
	void RecomputeBounds( const BVH *bvh, aabb* triangle_bounds );
	bool AABBIntersection( const Ray *r, const aabb &bb, float &tmin, float &tmax );
	void Print(BVH* bvh, uint depth);
  private:
	bool Traverse_Leaf(BVH *bvh, Ray *r, bool checkOcclusion);
	bool Traverse_Node(BVH *bvh, Ray *r, uint &depth, bool checkOcclusion);
	void Subdivide_Binned( BVH* bvh, aabb* triangle_bounds );
	void Subdivide_Median( BVH *bvh, aabb* triangle_bounds );
	void Subdivide_SAH( BVH *bvh, aabb* triangle_bounds );

	bool SAH( BVH *bvh, aabb* triangle_bounds, int &bestAxis, float &bestSplitLocation );
	void Divide( BVH *bvh, aabb* triangle_bounds, int &bestAxis, float &bestSplitLocation );
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

	void ConstructBVH( Triangle *triangles, uint triangleCount );
	void Print();

	inline bool Occludes( Ray *r )
	{
		uint depth = 0;
		return Traverse(r, depth, true);
	}
	inline bool Intersect( Ray* r, uint &depth )
	{
		return Traverse(r, depth, false);
	}
  private:
	inline bool Traverse( Ray *r, uint &depth, bool checkOcclusion )
	{
		if ( nr_triangles <= 0 ) return false;
		return root->Traverse( this, r, depth, checkOcclusion );
	}
};

}; // namespace AdvancedGraphics