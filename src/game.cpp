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
	sphere = new Sphere(vec3(0, 0, 3), 1, 0xff0000);
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

	// This can be done way better by changing the implementation of camera.
	// But for now this is the easiest way to comprehend and test.
	vec3 p0 = view->TopLeft();
	vec3 p1 = view->TopRight() - p0;
	vec3 p2 = view->BottomLeft() - p0;
	Color* buf = screen->GetBuffer();

	for (int y = 0; y < screen->GetHeight(); y++)
	for (int x = 0; x < screen->GetWidth(); x++)
	{
		float ux = (float)x / screen->GetWidth();
		float uy = (float)y / screen->GetHeight();
		vec3 dir = p0 + ux * p1 + uy * p2;
		
		Ray r = Ray(view->position, dir);
		sphere->Intersect(&r);

		*buf = r.color;
		buf++;
	}
}
