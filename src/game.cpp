#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"
#include "light.h"

// .obj loader
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"


// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init(int argc, char **argv)
{
	printf("Initializing Game\n");
	view = new Camera(0, 0, 0);

	// load lights
	nr_lights = 3;
	lights = new Light *[nr_lights] {
		new Light( { 0, 10, 0 }, { 0.05, 0.05, 0.05 } ),
		new Light( { 0, 10, 5 }, { 0.05, 0.05, 0.05 } ),
		new Light( { 0, 10, -5 }, { 0, 0.05, 0 } )
	};

	// load model
	switch (argc)
	{
		case 1: // No arguments
			nr_objects = 3;
			objects = new Primitive*[nr_objects] {
				new Sphere(vec3(0, 0, 10), 3, 0xff0000),
				new Plane(vec3(0, 1, 0), 2, 0xffffff),
				new Triangle(vec3(0, 0, 15), vec3(4, 5, 12), vec3(6, -6, 13), 0x0000ff)
			};
			break;
		case 2: {// An obj file
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string warn;
			std::string err;
			bool ret = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, argv[1] );
			if ( !warn.empty() )
				std::cout << warn << std::endl;
			if ( !err.empty() )
				std::cerr << err << std::endl;
			if ( !ret )
				exit( 1 );

			nr_objects = shapes.size();
			objects = new Primitive*[nr_objects];
			for (size_t s = 0; s < nr_objects; s++) {
				TriangleSoup* soup = new TriangleSoup(NULL, 0, 0x00ff00);
				TriangleSoup::FromTinyObj(soup, &attrib, &shapes[s].mesh);
				objects[s] = soup;
			}
			} break;
		default:
			std::cout << argc << " arguments is not accepted!" << std::endl;
			exit(1);
			break;
	}
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	printf("Shutting down Game\n");
}

struct Material
{
	Color color;
};

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

vec3 Normalize( vec3 v )
{
	float invLen = sqrtf( dot( v, v ) );
	return v * invLen;
}
float vDistance( vec3 first, vec3 second )
{
	return sqrt( pow( second.x - first.x, 2 ) + pow( second.y - first.y, 2 ) + pow( second.z - first.z, 2 ) * 1.0 );
}

Color Game::DirectIllumination( vec3 interPoint, vec3 normal )
{
	// accumulated color
	Color tmpColor = { 0, 0, 0 };
	float fac = 0;
	// send shadow ray to each light and add its color
	for ( size_t i = 0; i < nr_lights; i++ )
	{
		// compute origin and direction of shadow ray
		vec3 rayDirection = Normalize( lights[i]->position - interPoint );
		fac = dot( rayDirection, normal );
		vec3 rayOrigin = fac < 0 ? interPoint - ( 1e-3 * normal ) : interPoint + ( 1e-3 * normal );
		Ray shadowRay = Ray( rayOrigin, rayDirection );

		// find intersection of shadow ray, check if it is closest
		// TODO Intersect function that returns at the first time an intersection is found
		if ( Intersect( &shadowRay ) && shadowRay.t < vDistance( lights[i]->position, interPoint ) )
			continue;

		tmpColor.x += std::max( 0.0f, fac ) * lights[i]->color.x;
		tmpColor.y += std::max( 0.0f, fac ) * lights[i]->color.y;
		tmpColor.z += std::max( 0.0f, fac ) * lights[i]->color.z;
	}
	return tmpColor;
}

Color Game::Trace(Ray* r, uint depth)
{
	// TODO: handle depth value.

	if ( Intersect( r ) )
	{
		// TODO switch to determine wat material it is (diffuse, reflective, glass)
		// intersection point
		// TODO: get normal from r->obj
		vec3 interPoint = r->origin + r->t * r->direction;
		vec3 interNormal = r->obj->NormalAt( interPoint );

		vec3 ill = DirectIllumination( interPoint, interNormal );
		ill *= 1 / ( pow( r->t, 2 ) );
		// distance attenuation: return ( 1 / ( r->t * r->t ) ) * Color( red, green, blue );
		return { r->obj->color.x * ill.x, r->obj->color.y * ill.y, r->obj->color.z * ill.z };
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
