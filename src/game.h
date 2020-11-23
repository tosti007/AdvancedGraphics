#pragma once

#include "surface.h"
#include "camera.h"
#include "primitive.h"
#include "light.h"
#include "tiny_obj_loader.h"

namespace AdvancedGraphics {
struct float3
{
	float x, y, z;
};
class Game
{
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init(int argc, char **argv);
	void Shutdown();
	void Tick( float deltaTime );
	
	void MouseUp( int button ) { view->MouseUp(button); }
	void MouseDown( int button ) { view->MouseDown(button); }
	void MouseMove( int x, int y ) { view->MouseMove(x, y); }
	void KeyUp( int key, byte repeat ) { view->KeyUp(key, repeat); }
	void KeyDown( int key, byte repeat ) { view->KeyDown(key, repeat); }

	void LoadSkyBox();

	bool Intersect( Ray* r );
	vec3 DirectIllumination( vec3 interPoint, vec3 normal );
	Color Trace( Ray r, uint depth );
	void Print(size_t buflen, uint yline, const char *fmt, ...);

  private:
	Surface* screen;
	Camera* view;

	float3* skyPixels;
	int skyWidth;
	int skyHeight;

	Material** materials;
	uint nr_materials;
	Light** lights;
	uint nr_lights;
	Primitive** objects;
	uint nr_objects;

	void InitDefaultScene();
  	void InitFromTinyObj( char* filename );
};

}; // namespace AdvancedGraphics