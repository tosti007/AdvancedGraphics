#include <stdarg.h> // For variable number of arguments
#include <math.h>

#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"
#include "light.h"
#include "utils.h"

// .obj loader
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

void Game::InitDefaultScene()
{
	// load materials
	nr_materials = 4;
	materials = new Material[nr_materials] {
		Material(   0,   0, 0 ),	// Diffuse
		Material( 0.3,   0, 0 ),	// Diffuse & reflective
		Material( 0.2, 1.5, 0.15 ), // Glass
		Material(   1,   0, 0 )		// Mirror
	};

	nr_triangles = 0;
	nr_spheres = 6;
	spheres = new Sphere[nr_spheres] {
		Sphere( vec3( -3, -0.5, 5 ), 1.0f, 0xffffff, 2 ),
		Sphere( vec3( 0, -0.5, 5 ), 1.0f, 0xffffff, 2 ),
		Sphere( vec3( 3, -0.5, 5 ), 1.0f, 0xffffff, 2 ),
		Sphere( vec3( -3, -3.5, 5 ), 2.0f, 0x999999, 0 ),
		Sphere( vec3( 0, -3.5, 5 ), 2.0f, 0x999999, 0 ),
		Sphere( vec3( 3, -3.5, 5 ), 2.0f, 0x999999, 0 )
	};
}

void Game::InitFromTinyObj( const std::string filename )
{
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
	for (size_t t = 0; t < obj_materials.size(); t++)
	{
		auto mat = obj_materials[t];
		auto ior = mat.ior;
		if (ior == 1)
			ior = 0;
		materials[t] = Material(1 - mat.shininess, ior, mat.dissolve);
		std::string tname =  mat.diffuse_texname;
		auto search = textures.find(tname);
    	if (search == textures.end()) {
			std::string tname_full = basedir + tname;
			std::cout << "Load " << tname_full << std::endl;
			textures[tname] = new Surface(tname_full.c_str());
    	}
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
			Triangle::FromTinyObj(current, &attrib, &shapes[s].mesh, f, obj_materials, textures);
			current++;
		}
	}
}

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init(int argc, char **argv)
{
	printf("Initializing Game\n");
	view = new Camera(vec3(0, 0, 0), vec3(0, 0, 1));

	// load skybox
	sky = new SkyDome();

	// load lights
	// All lights should have atleast one color value != 0
	nr_lights = 1;
	lights = new Light *[nr_lights] {
		new SphereLight( vec3( 3, 3, 5 ), 1, Color( 50, 50, 50 ) )
	};

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

	for (uint i = 0; i < nr_spheres; i++)
	{
		if(spheres[i].material < 0)
			spheres[i].material = 0;
	}
	for (uint i = 0; i < nr_triangles; i++)
	{
		if(triangles[i].material < 0)
			triangles[i].material = 0;
	}

	#ifdef USERBVH 
		bvh = new BVH();
		bvh->ConstructBVH(triangles, nr_triangles);
		bvh->Print();
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
	// if any intersection found, return, don't need to know location
	for ( uint i = 0; i < nr_spheres; i++ )
	{
		if (spheres[i].Occludes( r ))
			return true;
	}

	for ( uint i = 0; i < nr_triangles; i++ )
	{
		if (triangles[i].Occludes( r ))
			return true;
	}
	return false;
}

bool Game::Intersect( Ray* r )
{
	bool found = false; 
	
	for (uint i = 0; i < nr_spheres; i++)
		found |= spheres[i].Intersect(r);

	#ifdef USERBVH 
		found |= bvh->Traverse(r);
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

Color Game::Trace(Ray r, uint depth)
{
	if (depth > MAX_NR_ITERATIONS)
		return Color(0, 0, 0);

	Light* light = IntersectLights( &r );

	// No intersection point found
	if ( !Intersect( &r ) )
	{
		if ( light != nullptr )
			return light->color;
		if (sky != nullptr)
			return sky->FindColor(r.direction);
		return SKYDOME_DEFAULT_COLOR;
	}

	// intersection point found
	vec3 interPoint = r.origin + r.t * r.direction;
	vec3 interNormal = r.obj->NormalAt( interPoint );
	float angle = -dot( r.direction, interNormal );
	bool backfacing = angle < 0.0f;
	if ( backfacing )
	{
		interNormal *= -1;
		angle *= -1;
	}

	bool reflect = false;
	bool refract = false;

	if (materials[r.obj->material].HasRefract())
	{
		refract = RandomFloat() < materials[r.obj->material].refractive;
	} else if (materials[r.obj->material].HasReflect())
	{
		reflect = RandomFloat() < materials[r.obj->material].speculative;
	}

	if (refract) {
		float n = backfacing ? materials[r.obj->material].refractive : 1.0f / materials[r.obj->material].refractive;
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
			return r.obj->ColorAt( interPoint ) * Trace( refractiveRay, depth + 1 );
		}
	}

	if (reflect)
	{
		// TODO: why does inverting the angle fix it?
		angle *= -1;
		// create reflect ray
		Ray reflectRay = Ray( interPoint, r.direction );
		reflectRay.Reflect( interPoint, interNormal, angle );
		reflectRay.Offset( 1e-3 );
		// Total Internal Reflection
		Color reflectCol = Trace( reflectRay, depth + 1 );
		return r.obj->ColorAt( interPoint ) * reflectCol;
	}

	// Random bounce
	vec3 random_dir = RandomPointOnHemisphere( 1, interNormal );
	Ray newRay = Ray( interPoint, random_dir );

	// irradiance
	Color ei = Trace( newRay, depth + 1 ) * dot( interNormal, random_dir );
	Color BRDF = r.obj->ColorAt( interPoint ) * INVPI;
	return PI * 2.0f * BRDF * ei;
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

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	printf("Game Tick\n");

	vec3 p0 = view->TopLeft();
	Pixel* buf = screen->GetBuffer();

	#ifdef USEVIGNETTING
		int dist_x_max = screen->GetWidth() / 2;
		int dist_y_max = screen->GetHeight() / 2;
		float dist_total_max = 1 / sqrtf(dist_x_max * dist_x_max + dist_y_max * dist_y_max);
	#endif
		
	for (int y = 0; y < screen->GetHeight(); y++)
	for (int x = 0; x < screen->GetWidth(); x++)
	{
		#ifdef SSAA
		// 4 rays with random offsett, then compute average
		Color color;
		for ( size_t i = 0; i < 4; i++ )
		{
			float u = ((float)x + randArray[i]) / screen->GetWidth();
			float v = ( (float)y + randArray[i+4] ) / screen->GetHeight();
			vec3 dir = p0 + u * view->right + v * view->down;
			dir.normalize();
			Ray r = Ray( view->position, dir );
			Color rayColor = Trace( r, 0 );
			color += rayColor;
		}
		color *= 0.25;
		float u = (float)x / screen->GetWidth();
		float v = (float)y / screen->GetHeight();
		#else
			float u = (float)x / screen->GetWidth();
			float v = (float)y / screen->GetHeight();
			vec3 dir = p0 + u * view->right + v * view->down;
			dir.normalize();

			Ray r = Ray( view->position, dir );

			Color color = Trace( r, 0 );
		#endif
		color.GammaCorrect();
		//color.ChromaticAbberation( { u, v } );

		#ifdef USEVIGNETTING
			color.Vignetting( ( x - screen->GetWidth() / 2 ), ( y - screen->GetHeight() / 2 ), dist_total_max );
		#endif

		*buf = color.ToPixel(*buf, unmoved_frames);

		buf++;
	}
	unmoved_frames++;

	// Write debug output
	Print(32, 0, "Pos: %f %f %f", view->position.x, view->position.y, view->position.z);
	
	Print(32, 1, "Dir: %f %f %f", view->direction.x, view->direction.y, view->direction.z);
	
	Print(32, 2, "Rgt: %f %f %f", view->right.x, view->right.y, view->right.z);
	
	Print(32, 3, "Dwn: %f %f %f", view->down.x, view->down.y, view->down.z);
	
	Print(32, 4, "FPS: %f", 1 / deltaTime);
}

void Game::CameraChanged()
{
	unmoved_frames = 0;
	screen->Clear(0);
}
