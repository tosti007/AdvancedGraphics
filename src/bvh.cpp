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
		depth++;
		bool found = nearNode.Traverse( bvh, r, depth );
		// early out
		if ( found && r->t <= bound )
			return found;

		// then far node
		found |= farNode.Traverse( bvh, r, depth );
		
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

void BVHNode::Subdivide( BVH *bvh )
{
	// Max number of primitives per leaf
	if ( count <= 3 || bvh->nr_nodes + 2 >= bvh->nr_nodes_max)
		return;

	#ifdef BINNING
	BinnedSAH( bvh );
	#else
	SAH( bvh );
	#endif

}

void BVHNode::BinnedSAH( BVH *bvh )
{
	int nr_bins = 8;

	// Calculate cost of node before split (For SAH)
	float currentCost = bounds.Area() * count;

	// Keep track current and lowest bounds, counts and costs
	float splitCost, lowestCost = currentCost;
	uint currentleftcount, currentrightcount, bestLeftCount, bestRightCount;
	aabb currentleftbox = aabb(), currentrightbox = aabb(), bestLeftBox = aabb(), bestRightBox = aabb();

	uint *idxs = new uint[count];
	uint *bestIdx = new uint[count];

	// Loop over every axis
	for ( size_t a = 0; a < 3; a++ )
	{
		float edgeMin = bounds.bmin[a];
		float edgeMax = bounds.bmax[a];
		float edgeLength = edgeMax - edgeMin;
		float binLength = edgeLength / nr_bins;

		// Loop over every bin on the axis
		for ( size_t b = 0; b < nr_bins; b++ )
		{
			// Get all triangles in the node
			for ( size_t k = 0; k < count; k++ )
				idxs[k] = bvh->indices[k];

			// (Re)set counts and bounds
			currentrightcount = 0;
			currentleftcount = 0;
			currentleftbox.Reset();
			currentrightbox.Reset();

			// Split at the binlocation
			float splitLocation = edgeMin + b * binLength;

			int index = -1;
			for ( size_t i = firstleft; i < firstleft + count; i++ )
			{
				const Triangle *tri = &bvh->triangles[bvh->indices[i]];
				aabb bb = aabb();
				GrowWithTriangle( &bb, tri );

				if (bb.Center(a) < splitLocation)
				{
					currentleftcount++;
					currentleftbox.Grow( bb );
					index++;
					// sort in place
					Swap( &idxs[index], &idxs[i - firstleft] );
				}
				else
				{
					currentrightcount++;
					currentrightbox.Grow( bb );
				}
			}

			// Compute area of the current left and right bounding boxes
			float leftArea = currentleftbox.Area();
			float rightArea = currentrightbox.Area();
			if ( isinf( leftArea ) )
				leftArea = 0;
			if ( isinf( rightArea ) )
				rightArea = 0;

			// Compute the cost of the current split
			// TODO: Add cost for extra aabb traversal
			float splitCost = rightArea * currentrightcount + leftArea * currentleftcount;

			// If found splitcost is lower than lowest cost up till now
			if (splitCost < lowestCost)
			{
				// Save the split
				bestRightBox = currentrightbox;
				bestLeftBox = currentleftbox;
				bestRightCount = currentrightcount;
				bestLeftCount = currentleftcount;
				for ( size_t k = 0; k < count; k++ )
					bestIdx[k] = idxs[k];

				lowestCost = splitCost;
			}
		}
	}

	// If found split is lower than the parents cost
	if (lowestCost < currentCost)
	{
		for ( size_t k = 0; k < count; k++ )
			bvh->indices[firstleft + k] = bestIdx[k];

		// Do actual split
		BVHNode *left, *right;

		// not a leaf node, so it doesn't have any triangles
		this->count = 0;
		// Save this
		int leftidx = bvh->nr_nodes;
		this->firstleft = firstleft; //bvh->nr_nodes;

		left = &bvh->pool[bvh->nr_nodes++];
		right = &bvh->pool[bvh->nr_nodes++];

		// Assign triangles to new nodes
		left->firstleft = firstleft;
		left->count = bestLeftCount;
		left->bounds = bestLeftBox;

		right->firstleft = firstleft + bestLeftCount;
		right->count = bestRightCount;
		right->bounds = bestRightBox;

		this->firstleft = leftidx;

		left->RecomputeBounds( bvh );
		right->RecomputeBounds( bvh );

		// Go in recursion on both child nodes
		left->Subdivide( bvh );
		right->Subdivide( bvh );
	}
}

void BVHNode::SAH( BVH *bvh )
{
	// Find longest axis for split, TODO: Binning
	int axis = this->bounds.LongestAxis();

	// Middle split, TODO: becomes better
	float splitLocation = bounds.Center( axis );

	// Counts and aabbs for new child nodes
	uint leftCount = 0;
	uint rightCount = 0;
	aabb leftbox, rightbox;
	leftbox.Reset();
	rightbox.Reset();

	// Move over all triangle indices inside the node
	for ( size_t i = firstleft; i < firstleft + count; i++ )
	{
		const Triangle *tri = &bvh->triangles[bvh->indices[i]];
		aabb bb = aabb();
		GrowWithTriangle( &bb, tri );

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
		left->Subdivide( bvh );
		right->Subdivide( bvh );
	}
}

void BVHNode::RecomputeBounds( const BVH* bvh )
{
	bounds.Reset();
	if (count > 0)
	{
		// Leaf node
		for ( size_t i = firstleft; i < firstleft + count; i++ )
			GrowWithTriangle(&bounds, &bvh->triangles[i]);
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

	// leave dummy value on location 0 for cache alignment
	nr_nodes = 1;
 
	root = &pool[nr_nodes++];
 	root->firstleft = 0;
 	root->count = triangleCount;
 	root->RecomputeBounds(this);

	root->Subdivide( this );
	printf( "Used number of nodes: %i\n", nr_nodes - 1 );
}

void BVH::Print()
{
	std::cout << "BVH:" << std::endl;
	root->Print(this, 1);
}
