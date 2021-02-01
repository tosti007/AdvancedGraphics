#pragma once

#include <map>

#include "surface.h"
#include "camera.h"
#include "primitive.h"
#include "light.h"
#include "skydome.h"
#include "bvh.h"
#include "tiny_obj_loader.h"

namespace AdvancedGraphics {

struct PixelData
{
	vec3 interNormal;
	vec3 firstIntersect;
	uint materialIndex;
	Color accumulated;
	Color albedo;
	Color illumination;

	inline PixelData() = default;
};

class Game
{
public:
	void SetTarget( Surface* surface );
	void Init(int argc, char **argv);
	void Shutdown();
	void Tick();
	
	void MouseUp( int button ) { if (view->MouseUp(button)) CameraChanged(); }
	void MouseDown( int button ) { if (view->MouseDown(button)) CameraChanged(); }
	void MouseMove( int x, int y ) { if (view->MouseMove(x, y)) CameraChanged(); }
	void KeyUp( int key, byte repeat ) { if (view->KeyUp(key, repeat)) CameraChanged(); }
	void KeyDown( int key, byte repeat ) { if (view->KeyDown(key, repeat)) CameraChanged(); }

	bool CheckOcclusion( Ray *r );
	bool Intersect( Ray* r, uint &depth );
	Light* IntersectLights( Ray* r );
	Color Sample( Ray r, bool specularRay, uint depth, uint pixelId );
	void GenerateGaussianKernel( float sigma );
	Color Filter( int pixelX, int pixelY, bool firstPass );
	void Print(size_t buflen, uint yline, const char *fmt, ...);

  private:
	float *kernel = nullptr;

	PixelData* pixelData = nullptr;
	Surface* screen;
	Camera* view;
	SkyDome* sky;

	#ifdef USEBVH 
		BVH* bvh;
	#endif

	Material* default_material;
	Material* materials;
	uint nr_materials;

	Light** lights;
	uint nr_lights;

	Sphere* spheres;
	uint nr_spheres;

	Triangle* triangles;
	uint nr_triangles;

	void InitDefaultScene();
  	void InitFromTinyObj( std::string filename );
	void InitSkyBox();

	uint unmoved_frames = 0;
	float frames_time = 0;
	float frames_fps = 0;
	void CameraChanged();
};

}; // namespace AdvancedGraphics