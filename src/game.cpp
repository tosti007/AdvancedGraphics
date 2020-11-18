#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	printf("Initializing Game\n");
	view = new Camera(0, 0, 0);
	sphere = new Sphere(vec3(0, 0, 10), 3, 0xff0000);
	floor = new Plane(vec3(0, 1, 0), 2, 0xffffff);
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	printf("Shutting down Game\n");
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	printf("Game Tick\n");

	vec3 p0 = view->TopLeft();
	Color* buf = screen->GetBuffer();

	for (int y = 0; y < screen->GetHeight(); y++)
	for (int x = 0; x < screen->GetWidth(); x++)
	{
		float ux = (float)x / screen->GetWidth();
		float uy = (float)y / screen->GetHeight();
		vec3 dir = p0 + ux * view->right + uy * view->down;
		dir.normalize();

		Ray r = Ray(view->position, dir);
		sphere->Intersect(&r);
		floor->Intersect(&r);

		*buf = r.color;
		buf++;
	}
}
