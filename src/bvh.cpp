#include "precomp.h" // include (only) this in every .cpp file
#include "bvh.h"

void GrowWithTriangle( aabb* bb, const Triangle* tri)
{
	bb->Grow( tri->p0 );
	bb->Grow( tri->p1 );
	bb->Grow( tri->p2 );
}

void BVHNode::Traverse( Ray r, BVH *bvh, Intersection &interPoint, int &depth )
{
	// if node is a leaf
	if ( count > 0 )
	{
		for ( size_t i = firstleft; i < ( firstleft + count ); i++ )
		{
			Triangle &tri = bvh->triangles[bvh->indices[i]];
			if (tri.Intersect( &r ))
			{
				
				interPoint.distance = r.t;
				interPoint.triangle = &tri;
				interPoint.location = r.origin + r.direction * interPoint.distance;
			}
		}
		return;
	}
	float tminL, tmaxL, tminR, tmaxR;

	bool intL = AABBIntersection( r, bvh->pool[firstleft].bounds, tminL, tmaxL );
	bool intR = AABBIntersection( r, bvh->pool[firstleft + 1].bounds, tminR, tmaxR );

	// if both sides are intersected, decide which is nearest
	if ( intL && intR )
	{
		bool leftIsNearNode = tminL < tminR;
		int bound = leftIsNearNode ? tminR : tminL;
		BVHNode nearNode = leftIsNearNode ? bvh->pool[firstleft] : bvh->pool[firstleft + 1];
		BVHNode farNode = !leftIsNearNode ? bvh->pool[firstleft + 1] : bvh->pool[firstleft];

		// first check nearest node
		nearNode.Traverse( r, bvh, interPoint, ++depth );
		// early out
		if ( interPoint.distance < bound )
			return;

		// then far node
		farNode.Traverse( r, bvh, interPoint, ++depth );
	}
	else if ( intL )
		bvh->pool[firstleft].Traverse( r, bvh, interPoint, ++depth );
	else if ( intR )
		bvh->pool[firstleft + 1].Traverse( r, bvh, interPoint, ++depth );
}

void Swap( uint *a, uint *b )
{
	uint t = *a;
	*a = *b;
	*b = t;
}

void BVHNode::Subdivide( BVH *bvh )
{
	// Max number of primitives per leaf
	if ( count <= 3 )
		return;

	// Calculate cost of node before split (For SAH)
	float currentCost = bounds.Area() * count;

	// Find longest axis for split, TODO: Binning
	int axis = this->bounds.LongestAxis();

	// Middle split, TODO: becomes better
	float splitLocation = bounds.Center( axis );

	// Counts and aabbs for new child nodes
	uint leftCount = 0;
	uint rightCount = 0;
	aabb leftbox = aabb();
	aabb rightbox = aabb();

	int index = firstleft - 1;
	// Move over all triangle indices inside the node
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		const Triangle *tri = bvh->triangles + bvh->indices[i];
		aabb bb = aabb();
		GrowWithTriangle(&bb, tri);

		float ac = bb.Center( axis );

		if ( ac < splitLocation )
		{
			leftCount++;
			leftbox.Grow( bb );
			index++;
			// Sort in place
			Swap( bvh->indices + index, bvh->indices + i );
		}
		else
		{
			rightCount++;
			rightbox.Grow( bb );
		}
	}
	// compute costs for new individual child nodes
	float leftArea = leftbox.Area();
	float rightArea = rightbox.Area();
	if ( isinf( leftArea ) )
		leftArea = 0;
	if ( isinf( rightArea ) )
		rightArea = 0;

	// TODO: Add cost for extra aabb traversal
	float splitCost = rightArea * rightCount + leftArea * leftCount;

	if (splitCost < currentCost)
	{
		// Do actual split
		BVHNode *left, *right;

		// not a leaf node, so it doesn't have any triangles
		this->count = 0;
		this->firstleft = bvh->nr_nodes;

		left = bvh->pool + bvh->nr_nodes++;
		right = bvh->pool + bvh->nr_nodes++;

		// Assign triangles to new nodes
		left->firstleft = firstleft;
		left->count = leftCount;
		left->bounds = leftbox;

		right->firstleft = firstleft + leftCount;
		right->count = rightCount;
		right->bounds = rightbox;

		// Go in recursion on both child nodes
		left->Subdivide( bvh );
		right->Subdivide( bvh );
	}
}

bool BVHNode::AABBIntersection( const Ray &r, const aabb &bb, float &tmin, float &tmax )
{
	vec3 invdir = { 1 / r.direction.x, 1 / r.direction.y, 1 / r.direction.z };
	vec3 vmin = (bb.bmin3 - r.origin) * invdir;
	vec3 vmax = (bb.bmax3 - r.origin) * invdir;

	tmax = std::min( std::min( std::max( vmin.x, vmax.x ), std::max( vmin.y, vmax.y ) ), std::max( vmin.z, vmax.z ) );

	if ( tmax < 0 )
		return false;

	tmin = std::max( std::max( std::min( vmin.x, vmax.x ), std::min( vmin.y, vmax.y ) ), std::min( vmin.z, vmax.z ) );
	if ( tmin > tmax )
		return false;

	return true;
}

void BVH::ConstructBVH( Triangle *triangles, uint triangleCount )
{
	printf( "Constructing BVH..." );
	// Create index array
	this->triangles = triangles;
	indices = new uint[triangleCount];

	// initial index values
	for ( size_t i = 0; i < triangleCount; i++ )
		indices[i] = i;

	// allocate space for BVH Nodes with max possible nodes
	nr_nodes_max = triangleCount * 2 - 1;
	pool = new BVHNode[nr_nodes_max];
	// leave dummy value on location 0 for cache alignment
	nr_nodes = 1;
 
	root = pool + nr_nodes++;
 	root->firstleft = 0;
 	root->count = triangleCount;
 	root->bounds = ComputeBounds( triangles, root->firstleft, root->count );

	root->Subdivide( this );
	printf( "Maximum number of nodes: %i", nr_nodes_max );
}

aabb BVH::ComputeBounds( const Triangle *triangles, int firstleft, uint count )
{
	aabb bb;
	for ( size_t i = firstleft; i < firstleft + count; i++ )
		GrowWithTriangle(&bb, &triangles[i]);
	return bb;
}