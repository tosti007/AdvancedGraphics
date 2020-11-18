#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"

// .obj loader
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

using namespace std;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	printf("Initializing Game\n");
	view = new Camera(0, 0, 0);
	sphere = new Sphere(vec3(0, 0, 10), 3, 0xff0000);
	floor = new Plane(vec3(0, 1, 0), 2, 0xffffff);
	trian = new Triangle(vec3(0, 0, 15), vec3(4, 5, 12), vec3(6, -6, 13), 0x0000ff);

	// load model
	string inputfile = "assets/cube.obj";
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;

	bool ret = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, inputfile.c_str() );

	if ( !warn.empty() )
		cout << warn << endl;
	if ( !err.empty() )
		cerr << err << endl;
	if ( !ret )
		exit( 1 );
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	printf("Shutting down Game\n");
}

bool Game::Intersect( Ray* r )
{
	bool i = false;
	// TODO: replace this with an iteration over all primitives.

	i |= sphere->Intersect(r);
	i |= floor->Intersect(r);
	i |= trian->Intersect(r);
	return i;
}

Color Game::Trace(Ray* r, uint depth)
{
	// TODO: handle depth value.

	if (Intersect(r))
	{
		// intersection point
		// vec3 I = r->origin + r->t * r->direction;

		// normal, depends on shape
		// vec3 N;
		return r->color;
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
