#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	printf("Initializing Game\n");
	view = new Camera(0, 0, 0);
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	printf("Shutting down Game\n");
}

static Sprite rotatingGun( new Surface( "assets/aagun.tga" ), 36 );
static int frame = 0;

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	// clear the graphics window
	screen->Clear( 0 );
	// print something in the graphics window
	screen->Print( "hello world", 2, 2, 0xffffff );
	// draw a sprite
	rotatingGun.SetFrame( frame );
	rotatingGun.Draw( screen, 100, 100 );
	if ( ++frame == 36 ) frame = 0;
}
