#pragma once

#include "vectors.h"
#include "ray.h"
#include "precomp.h"
#include "primitive.h"

namespace AdvancedGraphics
{

struct AABB
{
	vec3 tmin;
	vec3 tmax;
	AABB() = default;
	AABB( vec3 a, vec3 b )
	{ 
		tmin.x = a.x, tmin.y = a.y, tmin.z = a.z;
		tmax.x = b.x, tmax.y = b.y, tmax.z = b.z;
	}
	inline void Grow( const vec3 &v )
	{
		if ( v.x < tmin.x )
			tmin.x = v.x;
		else if ( v.x > tmax.x )
			tmax.x = v.x;

		if ( v.y < tmin.y )
			tmin.y = v.y;
		else if ( v.y > tmax.y )
			tmax.y = v.y;

		if ( v.z < tmin.z )
			tmin.z = v.z;
		else if ( v.z > tmax.z )
			tmax.z = v.z;
	}
	inline void Grow( const AABB &bb )
	{
		this->Grow( bb.tmin );
		this->Grow( bb.tmax );
	}
	float Area() const
	{
		vec3 dif = tmax - tmin;
		return std::max( 0.0f, dif.x * dif.y + dif.x * dif.z + dif.y * dif.z );
	}
	inline float Extend( const int axis ) const { return tmax[axis] - tmin[axis]; } // Does this work with vec3s?
	int LongestAxis() const
	{
		int a = 0;
		if ( Extend( 1 ) > Extend( 0 ) ) a = 1;
		if ( Extend( 2 ) > Extend( a ) ) a = 2;
		return a;
	}
	inline float Center( uint axis ) const { return ( tmin[axis] + tmax[axis] ) * 0.5f; }
};

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
	AABB bounds;
	uint firstleft; 
	uint count; // number of triangles
	void Traverse( Ray r, BVH *bvh, Intersection& i, int &depth );
	void Subdivide( BVHNode *pool, uint *indices, const Triangle *triangles );
	bool AABBIntersection( const Ray &r, const AABB &bb, float &tmin, float &tmax );
};

struct BVH
{
  public:
	BVHNode *pool;
	BVHNode *root;
	Triangle *triangles;
	uint *indices;
	void ConstructBVH( Triangle *triangles, uint triangleCount );
	static AABB ComputeBounds( const Triangle *triangles, int firstleft, int count );
};

}; // namespace AdvancedGraphics