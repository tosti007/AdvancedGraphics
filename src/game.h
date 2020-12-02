#pragma once

#include <map>

#include "surface.h"
#include "camera.h"
#include "primitive.h"
#include "light.h"
#include "skydome.h"
#include "tiny_obj_loader.h"

namespace AdvancedGraphics {

class Game
{
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init(int argc, char **argv);
	void Shutdown();
	void Tick( float deltaTime );
	
	void MouseUp( int button ) { if (view->MouseUp(button)) CameraChanged(); }
	void MouseDown( int button ) { if (view->MouseDown(button)) CameraChanged(); }
	void MouseMove( int x, int y ) { if (view->MouseMove(x, y)) CameraChanged(); }
	void KeyUp( int key, byte repeat ) { if (view->KeyUp(key, repeat)) CameraChanged(); }
	void KeyDown( int key, byte repeat ) { if (view->KeyDown(key, repeat)) CameraChanged(); }

	bool CheckOcclusion( Ray *r );
	Primitive* Intersect( Ray* r );
	Light* IntersectLights( Ray* r );
	Color DirectIllumination( vec3 interPoint, vec3 normal );
	Color Trace( Ray r, uint depth );
	void Print(size_t buflen, uint yline, const char *fmt, ...);

  private:
	Surface* screen;
	Camera* view;
	SkyDome* sky;

	Material** materials;
	uint nr_materials;
	std::map<std::string, Surface*> textures;

	Light** lights;
	uint nr_lights;
	Primitive** objects;
	uint nr_objects;

	void InitDefaultScene();
  	void InitFromTinyObj( std::string filename );
	void InitSkyBox();

	uint unmoved_frames = 0;
	void CameraChanged();

	Color RayTrace( Ray r, uint depth, Primitive* obj, vec3 interPoint, vec3 interNormal, float angle, bool backfacing );
	Color PathTrace( Ray r, uint depth, Primitive* obj, vec3 interPoint, vec3 interNormal, float angle, bool backfacing );
};

}; // namespace AdvancedGraphics