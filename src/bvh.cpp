#include "precomp.h" // include (only) this in every .cpp file
#include "bvh.h"

void GrowWithTriangle( aabb* bb, const Triangle* tri)
{
	bb->Grow( tri->p0 );
	bb->Grow( tri->p1 );
	bb->Grow( tri->p2 );
}

bool BVHNode::Traverse( BVH *bvh, Ray *r, uint &depth )
{
	// if node is a leaf
	if ( count > 0 )
	{
		assert(firstleft + count <= bvh->nr_triangles);
		bool found = false;
		for ( size_t i = 0; i < count; i++ )
		{
			found |= bvh->triangles[bvh->indices[firstleft + i]].Intersect( r );
		}
		return found;
	}

	assert(firstleft <= bvh->nr_nodes);
	assert(firstleft + 1 <= bvh->nr_nodes);

	float tminL, tmaxL, tminR, tmaxR;
	bool intL = AABBIntersection( r, bvh->pool[firstleft].bounds, tminL, tmaxL );
	bool intR = AABBIntersection( r, bvh->pool[firstleft + 1].bounds, tminR, tmaxR );

	// if both sides are intersected, decide which is nearest
	if ( intL && intR )
	{
		bool leftIsNearNode = tminL < tminR;
		float bound = leftIsNearNode ? tminR : tminL;
		BVHNode nearNode = leftIsNearNode ? bvh->pool[firstleft] : bvh->pool[firstleft + 1];
		BVHNode farNode = leftIsNearNode ? bvh->pool[firstleft + 1] : bvh->pool[firstleft];

		// first check nearest node
		bool found = nearNode.Traverse( bvh, r, ++depth );
		// early out
		if ( found && r->t <= bound )
			return found;

		// then far node
		found |= farNode.Traverse( bvh, r, ++depth );
		
		return found;
	}
	if (intL)
		return bvh->pool[firstleft].Traverse( bvh, r, ++depth );
	if (intR)
		return bvh->pool[firstleft + 1].Traverse( bvh, r, ++depth );

	return false;
}

void Swap( uint *a, uint *b )
{
	uint t = *a;
	*a = *b;
	*b = t;
}

void BVHNode::Subdivide( BVH *bvh, aabb* triangle_bounds )
{
	// Max number of primitives per leaf
	if ( count <= 3 || bvh->nr_nodes + 2 >= bvh->nr_nodes_max)
		return;

	Subdivide_Binned( bvh, triangle_bounds );
	// Subdivide_Median( bvh, triangle_bounds );
}

void BVHNode::Subdivide_Binned( BVH *bvh, aabb* triangle_bounds )
{
	const int nr_bins = 8;
	// Find longest axis and location for split
	/*
	// This should yield better values, but increases computational performance by a bit.
	aabb parentbounds; parentbounds.Reset();
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		aabb bb = triangle_bounds[bvh->indices[i]];
		parentbounds.Grow(bb.Center());
	}
	int axis = parentbounds.LongestAxis();
	float edgeMin = parentbounds.bmin[axis];
	float binLength = (parentbounds.bmax[axis] - edgeMin) / nr_bins;
	*/
	int axis = this->bounds.LongestAxis();
	float edgeMin = bounds.bmin[axis];
	float binLength = (bounds.bmax[axis] - edgeMin) / nr_bins;
	float binLengthInv = 1 / binLength;

	uint counts[nr_bins];
	aabb boxes[nr_bins];
	for (int i = 0; i < nr_bins; i++){
		counts[i] = 0;
		boxes[i].Reset();
	}
	
	// Populate step
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		aabb bb = triangle_bounds[bvh->indices[i]];
		int bin = (bb.Center(axis) - edgeMin) * binLengthInv;
		if ( bin >= nr_bins ) // For values where center == max bin edge
			bin = nr_bins - 1;
		counts[bin]++;
		boxes[bin].Grow(bb);
	}

	// Sweep step
	float splitCostBest = bounds.Area() * count;
	uint leftCountBest, rightCountBest;
	aabb leftBoxBest, rightBoxBest;
	int splitBinBest = -1;

	uint leftCount, rightCount;
	aabb leftBox, rightBox;
	leftCount = 0;
	leftBox.Reset();

	for ( int b = 0; b < nr_bins - 1; b++)
	{
		if (counts[b] == 0)
			continue;

		leftBox.Grow(boxes[b]);
		leftCount += counts[b];

		rightCount = 0;
		rightBox.Reset();

		for ( int b2 = b + 1; b2 < nr_bins; b2++)
		{
			rightBox.Grow(boxes[b2]);
			rightCount += counts[b2];
		}

		if (leftCount == 0 || rightCount == 0)
			continue;

		float splitCost = rightBox.Area() * rightCount + leftBox.Area() * leftCount;
		if ( splitCost < splitCostBest )
		{
			splitCostBest = splitCost;
			leftCountBest = leftCount;
			rightCountBest = rightCount;
			leftBoxBest = leftBox;
			rightBoxBest = rightBox;
			splitBinBest = b;
		}
	}

	if (splitBinBest < 0)
		return;

	// Divide step
	float splitLocation = edgeMin + (splitBinBest + 1) * binLength;
	leftCount = 0;
	// Move over all triangle indices inside the node
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		aabb bb = triangle_bounds[bvh->indices[i]];

		if ( bb.Center( axis ) < splitLocation )
		{
			Swap( &bvh->indices[firstleft + leftCount], &bvh->indices[i] );
			leftCount++;
		}
	}

	// Save this
	int leftidx = bvh->nr_nodes;

	// Do actual split
	BVHNode *left, *right;
	left = &bvh->pool[bvh->nr_nodes++];
	right = &bvh->pool[bvh->nr_nodes++];

	// Assign triangles to new nodes
	left->firstleft = firstleft;
	left->count = leftCountBest;
	left->bounds = leftBoxBest;

	right->firstleft = firstleft + leftCountBest;
	right->count = rightCountBest;
	right->bounds = rightBoxBest;

	this->count = 0;
	this->firstleft = leftidx;

	// Go in recursion on both child nodes
	left->Subdivide( bvh, triangle_bounds );
	right->Subdivide( bvh, triangle_bounds );
}

bool BVHNode::SAH( BVH *bvh, aabb* triangle_bounds, int &bestAxis, float &bestSplitLocation )
{
	bool foundLowerCost = false;
	// Counts and aabbs for new child nodes
	uint leftCount = 0;
	uint rightCount = 0;
	aabb leftbox, rightbox;

	float LowestCost = bounds.Area() * count;
	// Try every axis
	for ( size_t a = 0; a < 3; a++ )
	{
		// Try every vertex center as a split location
		for ( size_t i = firstleft; i < firstleft + count; i++ )
		{
			aabb bbForSplit = triangle_bounds[bvh->indices[i]];
			float splitLocation = bbForSplit.Center( a );
			leftbox.Reset();
			rightbox.Reset();
			leftCount = 0;
			rightCount = 0;
			// Divide every triangle on this split location
			for ( size_t j = firstleft; j < firstleft + count; j++ )
			{
				aabb bb = triangle_bounds[bvh->indices[j]];
				if ( bb.Center( a ) < splitLocation )
				{
					leftCount++;
					leftbox.Grow( bb );
				}
				else
				{
					rightCount++;
					rightbox.Grow( bb );
				}
			}
			// Early out if split does nothing
			if ( leftCount == 0 || rightCount == 0 )
				continue;

			float newCost = rightbox.Area() * rightCount + leftbox.Area() * leftCount;
			if ( newCost < LowestCost )
			{
				foundLowerCost = true;
				LowestCost = newCost;
				bestSplitLocation = splitLocation;
				bestAxis = a;
			}
		}
	}
	return foundLowerCost;
}

void BVHNode::Divide( BVH *bvh, aabb* triangle_bounds, int &axis, float &splitLocation )
{
	// Counts and aabbs for new child nodes
	uint leftCount = 0;
	uint rightCount = 0;
	aabb leftbox, rightbox;
	leftbox.Reset();
	rightbox.Reset();

	// Move over all triangle indices inside the node
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		aabb bb = triangle_bounds[bvh->indices[i]];

		if ( bb.Center( axis ) < splitLocation )
		{
			Swap( &bvh->indices[firstleft + leftCount], &bvh->indices[i] );
			leftCount++;
			leftbox.Grow( bb );
		}
		else
		{
			rightCount++;
			rightbox.Grow( bb );
		}
	}
	// Early out if split does nothing
	if ( leftCount == 0 || rightCount == 0 )
		return;

	// compute costs for new individual child nodes
	float leftArea = leftbox.Area();
	float rightArea = rightbox.Area();

	// TODO: Add cost for extra aabb traversal
	float splitCost = rightArea * rightCount + leftArea * leftCount;

	// Calculate cost of node before split (For SAH)
	float currentCost = bounds.Area() * count;
	if ( splitCost < currentCost )
	{
		// Save this
		int leftidx = bvh->nr_nodes;

		// Do actual split
		BVHNode *left, *right;
		left = &bvh->pool[bvh->nr_nodes++];
		right = &bvh->pool[bvh->nr_nodes++];

		// Assign triangles to new nodes
		left->firstleft = firstleft;
		left->count = leftCount;
		left->bounds = leftbox;

		right->firstleft = firstleft + leftCount;
		right->count = rightCount;
		right->bounds = rightbox;

		this->count = 0;
		this->firstleft = leftidx;

		// Go in recursion on both child nodes
		left->Subdivide( bvh, triangle_bounds );
		right->Subdivide( bvh, triangle_bounds );
	}
}

void BVHNode::Subdivide_Median( BVH *bvh, aabb* triangle_bounds )
{
	// Find longest axis for split, TODO: Binning
	int axis = this->bounds.LongestAxis();

	// Middle split, TODO: becomes better
	float splitLocation = bounds.Center( axis );

	Divide( bvh, triangle_bounds, axis, splitLocation );
}

void BVHNode::Subdivide_SAH( BVH *bvh, aabb* triangle_bounds )
{
	int axis;
	float splitLocation;
	if ( SAH( bvh, triangle_bounds, axis, splitLocation ) )
		Divide( bvh, triangle_bounds, axis, splitLocation );
}

void BVHNode::RecomputeBounds( const BVH* bvh, aabb* triangle_bounds )
{
	bounds.Reset();
	if (count > 0)
	{
		// Leaf node
		for ( size_t i = firstleft; i < firstleft + count; i++ )
			bounds.Grow(triangle_bounds[bvh->indices[i]]);
	}
	else
	{
		// Intermediate node
		bounds.Grow(bvh->pool[firstleft].bounds);
		bounds.Grow(bvh->pool[firstleft + 1].bounds);
	}
}

bool BVHNode::AABBIntersection( const Ray *r, const aabb &bb, float &tmin, float &tmax )
{
	vec3 invdir( 1 / r->direction.x, 1 / r->direction.y, 1 / r->direction.z );
	vec3 vmin = (bb.bmin3 - r->origin) * invdir;
	vec3 vmax = (bb.bmax3 - r->origin) * invdir;

	tmax = std::min( std::min( std::max( vmin.x, vmax.x ), std::max( vmin.y, vmax.y ) ), std::max( vmin.z, vmax.z ) );
	if ( tmax < 0 )
		return false;

	tmin = std::max( std::max( std::min( vmin.x, vmax.x ), std::min( vmin.y, vmax.y ) ), std::min( vmin.z, vmax.z ) );
	if ( tmin > tmax )
		return false;

	return true;
}

void BVHNode::Print(BVH* bvh, uint depth)
{
	// Indent with 2 spaces for each step
	for (uint i = 0; i < depth; i++)
		std::cout << "  ";

	if ( count == 0 ) 
	{
		// Not leaf
		std::cout << "Intermediate Node " << depth << std::endl;
		depth++;
		bvh->pool[firstleft].Print( bvh, depth );
		bvh->pool[firstleft + 1].Print( bvh, depth );
	} 
	else
	{
		std::cout << "Leaf Node " << depth << std::endl;
		depth++;
		for (uint i = 0; i < depth; i++)
			std::cout << "  ";
		for ( size_t i = 0; i < count; i++ )
			std::cout << bvh->indices[firstleft + i] << " ";
		std::cout << std::endl;
	}
}

void BVH::ConstructBVH( Triangle *triangles, uint triangleCount )
{
	printf( "Constructing BVH...\n" );
	// Create index array
	this->triangles = triangles;
	this->nr_triangles = triangleCount;
	//return;
	
	indices = new uint[triangleCount];

	// initial index values
	for ( size_t i = 0; i < triangleCount; i++ )
		indices[i] = i;

	// allocate space for BVH Nodes with max possible nodes
	nr_nodes_max = triangleCount * 2 - 1;
	pool = new BVHNode[nr_nodes_max];
	printf( "Maximum number of nodes: %i\n", nr_nodes_max - 1 );

	aabb *triangle_bounds = new aabb[triangleCount];
	for (uint t = 0; t < triangleCount; t++)
	{
		aabb *bb = triangle_bounds + t;
		const Triangle *tri = triangles + t;
		bb->Reset();
		GrowWithTriangle( bb, tri );
	}

	// leave dummy value on location 0 for cache alignment
	nr_nodes = 1;
 
	root = &pool[nr_nodes++];
 	root->firstleft = 0;
 	root->count = triangleCount;
 	root->RecomputeBounds(this, triangle_bounds);

	root->Subdivide( this, triangle_bounds );
	printf( "Used number of nodes: %i\n", nr_nodes - 1 );
}

void BVH::Print()
{
	std::cout << "BVH:" << std::endl;
	root->Print(this, 1);
}
