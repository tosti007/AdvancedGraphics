#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"

// .obj loader
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"


// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	printf("Initializing Game\n");
	view = new Camera(0, 0, 0);

	// load model
	std::string inputfile = "cube.obj";
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, inputfile.c_str() );
	if ( !warn.empty() )
		std::cout << warn << std::endl;
	if ( !err.empty() )
		std::cerr << err << std::endl;
	if ( !ret )
		exit( 1 );
	
	nr_objects = 3 + shapes.size();
	objects = new Primitive*[nr_objects];
	objects[nr_objects - 3] = new Sphere(vec3(0, 0, 10), 3, 0xff0000);
	objects[nr_objects - 2] = new Plane(vec3(0, 1, 0), 2, 0xffffff);
	objects[nr_objects - 1] = new Triangle(vec3(0, 0, 15), vec3(4, 5, 12), vec3(6, -6, 13), 0x0000ff);

	for (size_t s = 0; s < shapes.size(); s++) {
		TriangleSoup* soup = new TriangleSoup(NULL, 0, 0x00ff00);
		TriangleSoup::FromTinyObj(soup, &attrib, &shapes[s].mesh);
		objects[s] = soup;
	}
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	printf("Shutting down Game\n");
}
bool Game::RayTriIntersect( Ray* r, float v0, float v1, float v2)
{
	const float epsilon = 0.0000001;

	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 pvec = r->direction.cross( v0v2 );
	float det = v0v1.dot( pvec );

	// ray and triangle are parallel if det is close to 0
	// This should probaby be something smaller, but for now it will do
	if ( fabs( det ) < epsilon ) return false;

	float invDet = 1 / det;

	vec3 tvec = r->origin - v0;
	float u = tvec.dot( pvec ) * invDet;
	if ( u < 0 || u > 1 ) return false;

	vec3 qvec = tvec.cross( v0v1 );
	float v = r->direction.dot( qvec ) * invDet;
	if ( v < 0 || u + v > 1 ) return false;

	float t = v0v2.dot( qvec ) * invDet;

	if ( t >= r->t || t <= 0 ) return false;
	r->t = t;
	// TODO save triangle to ray?
	//r->obj = this;
	return true;
}

struct Material
{
	Color color;
};

bool Game::NearestIntersect( Ray *r, tinyobj::index_t nearestIdx )
{
	float nearest = INFINITY;
	bool ret = false;
	// Loop over shapes
	for ( size_t s = 0; s < shapes.size(); s++ )
	{
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for ( size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++ )
		{
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for ( size_t v = 0; v < fv; v++ )
			{
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				if (RayTriIntersect(r, vx, vy, vz) && r->t < nearest)
				{
					ret = true;
					nearest = r->t;
					nearestIdx = idx;
				}
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}
	return ret;
}

bool Game::Intersect( Ray* r )
{
	bool found = false;
	// TODO: replace this with an iteration over all primitives.

	for (uint i = 0; i < nr_objects; i++)
	{
		found |= objects[i]->Intersect(r);
	}

	return found;
}

Color Game::Trace(Ray* r, uint depth)
{
	// TODO: handle depth value.

	if ( Intersect( r ) )
	{
		// intersection point
		//vec3 interPoint = r->origin + r->t * r->direction;
		// vertex normals
		//tinyobj::real_t nx = attrib.normals[3 * nearestIdx.normal_index + 0];
		//tinyobj::real_t ny = attrib.normals[3 * nearestIdx.normal_index + 1];
		//tinyobj::real_t nz = attrib.normals[3 * nearestIdx.normal_index + 2];
		// tex coords
		//tinyobj::real_t tx = attrib.texcoords[2 * nearestIdx.texcoord_index + 0];
		//tinyobj::real_t ty = attrib.texcoords[2 * nearestIdx.texcoord_index + 1];
		// vertex colors
		//tinyobj::real_t red = attrib.colors[3 * nearestIdx.vertex_index + 0];
		//tinyobj::real_t green = attrib.colors[3 * nearestIdx.vertex_index + 1];
		//tinyobj::real_t blue = attrib.colors[3 * nearestIdx.vertex_index + 2];

		//return ( 1 / ( r->t * r->t ) ) * Color( red, green, blue );
		//return Color(red, green, blue);
		return r->obj->color;
	} else {
		return Color(0); // maybe add skydome here
	}
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	printf("Game Tick\n");

	vec3 p0 = view->TopLeft();
	Pixel* buf = screen->GetBuffer();

	for (int y = 0; y < screen->GetHeight(); y++)
	for (int x = 0; x < screen->GetWidth(); x++)
	{
		float u = (float)x / screen->GetWidth();
		float v = (float)y / screen->GetHeight();
		vec3 dir = p0 + u * view->right + v * view->down;
		dir.normalize();

		Ray r = Ray(view->position, dir);

		Color color = Trace( &r, 1 );

		*buf = ColorToPixel(color);
		buf++;
	}

	// Write debug output
	char buffer [32];
	sprintf (buffer, "Pos: %f %f %f", view->position.x, view->position.y, view->position.z);
	screen->Print(buffer, 2, 2, 0xffff00);

	sprintf (buffer, "Dir: %f %f %f", view->direction.x, view->direction.y, view->direction.z);
	screen->Print(buffer, 2, 9, 0xffff00);

	// Timer
	sprintf( buffer, "FPS: %f", 1 / deltaTime);
	screen->Print( buffer, 2, 16, 0xffff00 );
}
