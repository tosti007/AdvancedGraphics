#pragma once

#include "surface.h"
#include "camera.h"

namespace AdvancedGraphics {

class Game
{
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init();
	void Shutdown();
	void Tick( float deltaTime );
	void MouseUp( int button ) { view->MouseUp(button); }
	void MouseDown( int button ) { view->MouseDown(button); }
	void MouseMove( int x, int y ) { view->MouseMove(x, y); }
	void KeyUp( int key, byte repeat ) { view->KeyUp(key, repeat); }
	void KeyDown( int key, byte repeat ) { view->KeyDown(key, repeat); }
private:
	Surface* screen;
	Camera* view;
};

}; // namespace AdvancedGraphics