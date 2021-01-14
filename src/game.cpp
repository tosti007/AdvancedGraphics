#include <stdarg.h> // For variable number of arguments
#include <math.h>
#include <chrono>

#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"
#include "light.h"
#include "utils.h"
#include "timer.h"

// .obj loader
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

void Game::InitDefaultScene()
{
	// materials
	nr_materials = 6;
	materials = new Material[nr_materials] {
		Material(0, 0, 1, Color(1, 1, 1), nullptr), // wall
		Material(1, 0, 1, Color(1, 1, 1), nullptr), // mirror
		Material(0.5f, 0.5f, 1.5f, Color(1, 1, 1), nullptr), // glass
		Material(0, 0, 1, Color(1, 0, 0), nullptr), // red
		Material(0, 0, 1, Color(0, 1, 0), nullptr), // green
		Material(0, 0, 1, Color(0, 0, 1), nullptr), // blue
	};

	// triangles
	float room_width = 6;
	float room_height = 3;
	// loa = links, onder, achter; rbv = rechts, boven, voor
	vec3 loa = vec3(0, 0, 0), lov = vec3(0, 0, room_width);
	vec3 lba = vec3(0, room_height, 0), lbv = vec3(0, room_height, room_width);
	vec3 roa = vec3(room_width, 0, 0), rov = vec3(room_width, 0, room_width);
	vec3 rba = vec3(room_width, room_height, 0), rbv = vec3(room_width, room_height, room_width);

	nr_triangles = 12;
	triangles = new Triangle[nr_triangles] {
		Triangle(loa, lov, roa, 0), // floor 1
		Triangle(lov, roa, rov, 0), // floor 2
		Triangle(lba, lbv, rba, 0), // roof 1
		Triangle(lbv, rba, rbv, 0), // roof 2
		Triangle(loa, lba, lov, 0), // wall left 1
		Triangle(lba, lov, lbv, 0), // wall left 2
		Triangle(roa, rba, rov, 0), // wall right 1
		Triangle(rba, rov, rbv, 0), // wall right 2
		Triangle(loa, lba, roa, 0), // wall back 1
		Triangle(lba, roa, rba, 0), // wall back 2
		Triangle(lov, lbv, rov, 0), // wall front 1
		Triangle(lbv, rov, rbv, 0), // wall front 2
	};

	// spheres
	float radius = 0.5f;

	nr_spheres = 3;
	spheres = new Sphere[nr_spheres] {
		Sphere(vec3(2 + 0, radius + 0.2f, 2.5f + 0), radius, 3),
		Sphere(vec3(2 + 1, radius + 0.2f, 2.5f + 1), radius, 4),
		Sphere(vec3(2 + 2, radius + 0.2f, 2.5f + 2), radius, 5),
	};

	view = new Camera(vec3(room_width/2, 1, 0.3f), vec3(0, 0, 1));
	sky = nullptr;

	nr_lights = 1;
	lights = new Light *[nr_lights] {
		new SphereLight( vec3( room_width/2, room_height, room_width/2 ), 0.5f, Color( 200, 200, 200 ) )
	};
}

void Game::InitFromTinyObj( const std::string filename )
{
	view = new Camera( vec3( -15, -10, -0.1 ), vec3( 1, 0, 0 ) );


	// load skybox
	sky = new SkyDome();

	// load lights
	// All lights should have atleast one color value != 0
	nr_lights = 1;
	lights = new Light *[nr_lights] {
		new SphereLight( vec3( -5, 10, 0 ), 8, Color( 50, 50, 50 ) )
	};

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> obj_materials;
	std::string warn;
	std::string err;

	std::string basedir;
	size_t found = filename.find_last_of("/\\");
	if (found == std::string::npos) {
		basedir = std::string("");
	} else {
		basedir = filename.substr(0,found + 1);
	}

	bool ret = tinyobj::LoadObj( &attrib, &shapes, &obj_materials, &warn, &err, filename.c_str(), basedir.c_str(), true, false );
	if ( !warn.empty() )
		std::cout << warn << std::endl;
	if ( !err.empty() )
		std::cerr << err << std::endl;
	if ( !ret )
		exit( 1 );

	std::cout << "Loading texture maps" << std::endl;

	// load materials
	nr_materials = obj_materials.size();
	materials = new Material[nr_materials];
	Material* current_mat = materials;
	for (size_t t = 0; t < obj_materials.size(); t++)
	{
		Material::FromTinyObj(current_mat, basedir, obj_materials[t]);
		current_mat++;
	}

	nr_spheres = 0;
	nr_triangles = 0;
    for (size_t s = 0; s < shapes.size(); s++)
		nr_triangles += shapes[s].mesh.indices.size() / 3;

	triangles = new Triangle[nr_triangles];
	Triangle* current = triangles;

	for (size_t s = 0; s < shapes.size(); s++)
	{
		for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
		{
			Triangle::FromTinyObj(current, &attrib, &shapes[s].mesh, f, obj_materials);
			current++;
		}
	}
}

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::SetTarget( Surface* surface )
{ 
	screen = surface;
	if (colors != nullptr)
		free(colors);
	colors = new Color[screen->GetWidth() * screen->GetHeight()];
	CameraChanged();
}

void Game::Init(int argc, char **argv)
{
	printf("Initializing Game\n");
	default_material = new Material();

	// load model
	switch (argc)
	{
		case 1: // No arguments
			InitDefaultScene();
			break;
		case 2: // An obj file
			InitFromTinyObj(std::string(argv[1]));
			break;
		default:
			std::cout << argc << " arguments not accepted!" << std::endl;
			exit(1);
			break;
	}

	#ifdef USEBVH 
	if ( nr_triangles > 0 )
	{
		bvh = new BVH();

		timer::TimePoint t = timer::get();
		bvh->ConstructBVH( triangles, nr_triangles );
		std::cout << "Construction time: " << timer::elapsed(t) << " ms." << std::endl;

		if (bvh->nr_nodes < 100 && nr_triangles < 100)
			bvh->Print();
	}
	#endif
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	printf("Shutting down Game\n");
}

bool Game::CheckOcclusion( Ray *r )
{
	// If any intersection found, return, don't need to know location
	// Check Spheres
	for ( uint i = 0; i < nr_spheres; i++ )
	{
		if (spheres[i].Occludes( r ))
			return true;
	}
	// Check triangles
	uint depth = 0;
	bvh->Traverse( r, depth, true );
	// Check lights
	for ( size_t i = 0; i < nr_lights; i++ )
	{
		if ( lights[i]->Occludes( r ) )
			return true;
	}
	return false;
}

bool Game::Intersect( Ray* r, uint &depth )
{
	bool found = false; 
	
	for (uint i = 0; i < nr_spheres; i++)
		found |= spheres[i].Intersect(r);

	#ifdef USEBVH 
		found |= bvh->Traverse(r, depth, false);
	#else
		for (uint i = 0; i < nr_triangles; i++)
			found |= triangles[i].Intersect(r);
	#endif

	return found;
}

Light* Game::IntersectLights( Ray* r )
{
	Light* found = nullptr;

	for (uint i = 0; i < nr_lights; i++)
	{
		if(lights[i]->Intersect(r))
			found = lights[i];
	}

	return found;
}

Color Game::DirectIllumination( vec3 interPoint, vec3 normal )
{
	// accumulated color
	Color total( 0 );
	Ray shadowRay(interPoint, vec3(0));
	float fac;
	// send shadow ray to each light and add its color
	for ( size_t l = 0; l < NR_LIGHT_SAMPLES; l++ )
	{
		// TODO: take size of light into account -> higher chance for lights that are closer or bigger
		size_t i = RandomIndex(nr_lights);
		// compute origin and direction of shadow ray
		shadowRay.direction = lights[i]->PointOnLight() - shadowRay.origin;
		shadowRay.t = shadowRay.direction.length();
		shadowRay.direction *= (1 / shadowRay.t);

		fac = dot( shadowRay.direction, normal );

		// This would mean no additional color so let's early out.
		// TODO: use epsilon?
		if (fac <= 0)
			continue;

		// TODO: Check the normal of the light and the shadowray direction, their dot should also be >0
		// fac_light = ???
		float fac_light = 1;

		if (fac_light <= 0)
			continue;

		// find intersection of shadow ray, check if it is between the light and object
		if ( CheckOcclusion( &shadowRay ) )
			continue;

		// TODO: Find some way of getting the Scene.LIGHTAREA.
		float light_area = 1;

		// angle * distance attenuation * color
		Color thislight = (fac / ( shadowRay.t * shadowRay.t )) * lights[i]->color;
		thislight *= INVPI * nr_lights * fac_light * light_area;
		total += thislight;
	}
	return (1 / NR_LIGHT_SAMPLES) * total;
}

Color Game::Sample(Ray r, uint depth)
{
	if (depth > MAX_NR_ITERATIONS)
		return Color(0, 0, 0);

	Light* light = IntersectLights( &r );

	uint bvhDepth = 0;
	bool found = Intersect( &r, bvhDepth );

	#ifdef VISUALIZEBVH
		return { 0, std::min( 0.02f * bvhDepth, 1.0f ), 0 };
	#endif

	// No intersection point found
	if ( !found )
	{
		if ( light != nullptr )
		#ifdef USENEE
			return Color(0, 0, 0);
		#else
			return light->color;
		#endif
		if (sky != nullptr)
			return sky->FindColor(r.direction);
		return SKYDOME_DEFAULT_COLOR;
	}

	// As al our debuging objects are close, 1000 is a safe value.
	// assert(r.t <= 1000);
	assert(r.obj != nullptr);
	assert(r.obj->material >= -1);
	assert(r.obj->material < (int)nr_materials);

	// intersection point found
	vec3 interPoint = r.origin + r.t * r.direction;
	vec3 interNormal = r.obj->NormalAt( interPoint );
	Color BRDF = r.obj->ColorAt( materials, interPoint ) * INVPI;
	float angle = -dot( r.direction, interNormal );
	bool backfacing = angle < 0.0f;
	if ( backfacing )
	{
		interNormal *= -1;
		angle *= -1;
	}

	Material* mat = default_material;
	if (r.obj->material >= 0) 
		mat = &materials[r.obj->material];

	bool reflect = false;
	bool refract = false;

	if (mat->HasRefraction())
	{
		refract = RandomFloat() < mat->GetRefraction();
	} else if (mat->HasReflection())
	{
		reflect = RandomFloat() < mat->GetReflection();
	}

	if (refract) {
		float n = mat->GetIoR();
		if (backfacing) n = 1.0f / n;
		float k = 1 - ( n * n * ( 1 - angle * angle ) );

		if (k < 0)
		{
			reflect = true;
		} else {
			// Calculate the refractive ray, and its color
			vec3 refractDir = n * r.direction + interNormal * ( n * angle - sqrtf( k ) );
			refractDir.normalize();
			Ray refractiveRay( interPoint, refractDir );
			refractiveRay.Offset( 1e-3 );
			return r.obj->ColorAt( materials, interPoint ) * Sample( refractiveRay, depth + 1 );
		}
	}

	if (reflect)
	{
		// Total Internal Reflection
		angle *= -1; // TODO: why does inverting the angle fix it?
		Ray reflectRay = Ray( interPoint, r.direction );
		reflectRay.Reflect( interPoint, interNormal, angle );
		reflectRay.Offset( 1e-3 );
		Color reflectCol = Sample( reflectRay, depth + 1 );
		return r.obj->ColorAt( materials, interPoint ) * reflectCol;
	}

	#ifdef USENEE
	// Direct light for NEE
	Light &rLight = *lights[RandomIndex( nr_lights )];
	vec3 rLightPoint = rLight.PointOnLight();
	
	vec3 rLightDir = normalize(rLightPoint - interPoint);
	vec3 rLightNormal = rLight.NormalAt(rLightPoint);
	float rLightDist = ( rLightPoint - interPoint ).length();
	float rLightArea = rLight.Area();

	Ray rLightRay = Ray( interPoint, rLightDir );
	rLightRay.t = rLightDist;
	Color rLightCol = 0x000000;
	if (interNormal.dot(rLightDir) > 0 && rLightNormal.dot(-rLightDir) && !CheckOcclusion(&rLightRay))
	{
		float solidAngle = ( ( rLightNormal.dot(-rLightDir)) * rLightArea) / (rLightDist * rLightDist);
		rLightCol = rLight.color * solidAngle * BRDF * interNormal.dot( rLightDir );
	}
	#endif

	// Random bounce
	Ray randomRay = Ray( interPoint, RandomPointOnHemisphere( 1, interNormal ) );
	randomRay.Offset(1e-3);

	// irradiance
	Color ei = Sample( randomRay, depth + 1 ) * dot( interNormal, randomRay.direction );
	Color result = PI * 2.0f * BRDF * ei;

	#ifdef USENEE
	result += rLightCol;
	#endif

	return result;
}

void Game::Print(size_t buflen, uint yline, const char *fmt, ...) {
	char buf[128];
	va_list va;
    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);
	screen->Print(buf, 2, 2 + yline * 7, 0xffff00);
}

// floats for anti aliasing
float randArray[8] = {
	Rand( 1 ),
	Rand( 1 ),
	Rand( 1 ),
	Rand( 1 ),
	Rand( 1 ),
	Rand( 1 ),
	Rand( 1 ),
	Rand( 1 ) };

Ray ComputePrimaryRay(Surface* screen, Camera* view, int x, int y, size_t i, size_t j)
{
	float u = x, v = y;

#ifdef SSAA
	u += randArray[i];
	v += randArray[i + j];
#endif

	u /= screen->GetWidth();
	v /= screen->GetHeight();
	vec3 dir = view->topLeft + u * view->right + v * view->down;
	dir.normalize();
	return Ray( view->position, dir );
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick()
{
	timer::TimePoint dt = timer::get();

	#ifdef USEVIGNETTING
		int dist_x_max = screen->GetWidth() / 2;
		int dist_y_max = screen->GetHeight() / 2;
		float dist_total_max = 1 / sqrtf(dist_x_max * dist_x_max + dist_y_max * dist_y_max);
	#endif
	
	unmoved_frames++;
	#pragma omp parallel for schedule( dynamic ) num_threads(8)
	for (int y = 0; y < screen->GetHeight(); y++)
	for (int x = 0; x < screen->GetWidth(); x++)
	{
		uint id = x + y * screen->GetWidth();

		#ifdef SSAA
			// 4 rays with random offsett, then compute average
			Color color;
			for ( size_t i = 0; i < 4; i++ )
			{
				Ray r = ComputePrimaryRay(screen, view, x, y, i, i + 4);
				Color rayColor = Sample( r, 0 );
				color += rayColor;
			}
			color *= 0.25;
		#else
			Ray r = ComputePrimaryRay(screen, view, x, y, 0, 0);
			Color color = Sample( r, 0 );
		#endif
		color.GammaCorrect();
		//color.ChromaticAbberation( { u, v } );

		#ifdef USEVIGNETTING
			color.Vignetting( ( x - screen->GetWidth() / 2 ), ( y - screen->GetHeight() / 2 ), dist_total_max );
		#endif

		colors[id] += color;
		screen->GetBuffer()[id] = colors[id].ToPixel( unmoved_frames );
	}

	// Kernel filter
	for ( int y = 1; y < screen->GetHeight() - 1; y++ )
		for (int x = 1; x < screen->GetWidth() - 1; x++)
		{
			uint id = x + y * screen->GetWidth();
			Color tmp = 0x000000;
			tmp += 0.05 * colors[(x - 1) + (y - 1) * screen->GetWidth()];
			tmp += 0.15 * colors[(x) + (y - 1) * screen->GetWidth()];
			tmp += 0.05 * colors[(x + 1) + (y - 1) * screen->GetWidth()];
			tmp += 0.15 * colors[(x - 1) + (y) * screen->GetWidth()];
			tmp += 0.2 * colors[(x) + (y) * screen->GetWidth()];
			tmp += 0.15 * colors[(x + 1) + (y) * screen->GetWidth()];
			tmp += 0.05 * colors[(x - 1) + (y + 1) * screen->GetWidth()];
			tmp += 0.15 * colors[(x) + (y + 1) * screen->GetWidth()];
			tmp += 0.05 * colors[(x + 1) + (y + 1) * screen->GetWidth()];

			screen->GetBuffer()[id] = tmp.ToPixel( unmoved_frames );
		}

	// Write debug output
	Print(32, 0, "Pos: %f %f %f", view->position.x, view->position.y, view->position.z);
	
	Print(32, 1, "Dir: %f %f %f", view->direction.x, view->direction.y, view->direction.z);
	
	Print(32, 2, "Rgt: %f %f %f", view->right.x, view->right.y, view->right.z);
	
	Print(32, 3, "Dwn: %f %f %f", view->down.x, view->down.y, view->down.z);
	
	float elapsed = timer::elapsed(dt);
	frames_time += elapsed;
	if (frames_time > 500)
	{
		frames_fps = 1000.0f / elapsed;
		frames_time = 0;
		std::cout << "FPS: " << frames_fps << std::endl;
	}

	Print(32, 4, "FPS: %f", frames_fps);
}

void Game::CameraChanged()
{
	unmoved_frames = 0;
	auto scr_buf = screen->GetBuffer();
	int max = screen->GetWidth() * screen->GetHeight();
	for ( int i = 0; i < max; i++ )
	{
		scr_buf[i] = 0;
		colors[i].r = 0.0f;
		colors[i].g = 0.0f;
		colors[i].b = 0.0f;
	}
}
