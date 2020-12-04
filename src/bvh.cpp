#include "bvh.h"

void BVHNode::Traverse( Ray r, BVH *bvh, vec3 &i, int &depth )
{
}

void Swap( uint *a, uint *b )
{
	uint t = *a;
	*a = *b;
	*b = t;
}

// forward declaration
uint poolPtr, nodeCount;
void BVHNode::Subdivide( BVHNode *pool, uint *indices, const Triangle *triangles )
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

	// Counts and AABBs for new child nodes
	uint leftCount = 0;
	uint rightCount = 0;
	AABB leftbox = AABB();
	AABB rightbox = AABB();

	int index = firstleft - 1;
	// Move over all triangle indices inside the node
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		const auto &tri = triangles[indices[i]];
		AABB bb = AABB();
		bb.Grow( tri.p0 );
		bb.Grow( tri.p1 );
		bb.Grow( tri.p2 );

		float ac = bb.Center( axis );

		if ( ac < splitLocation )
		{
			leftCount++;
			leftbox.Grow( bb );
			index++;
			// Sort in place
			Swap( &indices[index], &indices[i] );
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
		int leftidx = poolPtr;
		// notl eaf node, so it doesn't have any triangles
		this->count = 0;
		left = &pool[poolPtr++];
		right = &pool[poolPtr++];

		// Assign triangles to new nodes
		left->firstleft = firstleft;
		left->count = leftCount;
		left->bounds = leftbox;
		nodeCount++;

		right->firstleft = firstleft + leftCount;
		right->count = rightCount;
		right->bounds = rightbox;
		nodeCount++;

		this->firstleft = leftidx;

		// Go in recursion on both child nodes
		left->Subdivide( pool, indices, triangles );
		right->Subdivide( pool, indices, triangles );
	}
}

bool BVHNode::AABBIntersection( const Ray &r, const AABB &bb, float &tmin, float &tmax )
{
	return false;
}

void BVH::ConstructBVH( Triangle *triangles, uint triangleCount )
{
	printf( "Constructing BVH..." );
	// Create index array
	this->triangles = triangles;
	indices = new uint[triangleCount];

	// initial index values
	for ( int i = 0; i < triangleCount; i++ )
		indices[i] = i;

	// allocate space for BVH Nodes with max possible nodes
	pool = new BVHNode[triangleCount * 2 - 1];
	root = &pool[0];
	// leave dummy value on location 1 for cache alignment
	poolPtr = 2;

	root->firstleft = 0;
	root->count = triangleCount;
	root->bounds = ComputeBounds( triangles, root->firstleft, root->count );
	root->Subdivide( pool, indices, triangles );
	printf( "Number of nodes: %f", nodeCount );
}

AABB BVH::ComputeBounds( const Triangle *triangles, int firstleft, int count )
{
	AABB bb;
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		bb.Grow( triangles[i].p0 );
		bb.Grow( triangles[i].p1 );
		bb.Grow( triangles[i].p2 );
	}
	return bb;
}