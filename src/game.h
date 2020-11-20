#pragma once

#include "surface.h"
#include "camera.h"
#include "primitive.h"
#include "tiny_obj_loader.h"

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

	bool Intersect( Ray* r );
	Color Trace( Ray* r, uint depth );
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

  private:
	Surface* screen;
	Camera* view;
	Primitive** objects;
	uint nr_objects;
};

}; // namespace AdvancedGraphics