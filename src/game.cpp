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
	nr_objects = 7;
	objects = new Primitive *[nr_objects] {
		//new Sphere( vec3( -3, 2, 10 ), 2.5, 0xffffff, materials[1] ),
		//new Sphere( vec3( 3, 2, 10 ), 2.5, 0xffffff, materials[1] ),
		new Sphere(vec3( -3, -0.5, 5 ), 1.0f, 0xff0000, materials[2]),
		new Sphere(vec3( 0, -0.5, 5 ), 1.0f, 0x00ff00, materials[2]),
		new Sphere(vec3( 3, -0.5, 5 ), 1.0f, 0x0000ff, materials[2]),
		new Sphere( vec3( -3, -3.5, 5 ), 2.0f, 0x999999, materials[0] ),
		new Sphere( vec3( 0, -3.5, 5 ), 2.0f, 0x999999, materials[0] ),
		new Sphere( vec3( 3, -3.5, 5 ), 2.0f, 0x999999, materials[0] ),
		new Plane( vec3( 0, 1, 0 ), 2.0f, 0x11ffff, materials[1]),
		//new Triangle( vec3( 0, 0, 15 ), vec3( 4, 5, 12 ), vec3( 6, -6, 13 ), 0x0000ff, materials[1] )
	};
}

void Game::InitFromTinyObj( const std::string filename )
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	std::string basedir;
	size_t found = filename.find_last_of("/\\");
	if (found == std::string::npos) {
		basedir = std::string("");
	} else {
		basedir = filename.substr(0,found + 1);
	}

	bool ret = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, filename.c_str(), basedir.c_str(), true, false );
	if ( !warn.empty() )
		std::cout << warn << std::endl;
	if ( !err.empty() )
		std::cerr << err << std::endl;
	if ( !ret )
		exit( 1 );

	std::cout << "Loading texture maps" << std::endl;

	for (size_t t = 0; t < materials.size(); t++)
	{
		std::string tname =  materials[t].diffuse_texname;
		auto search = textures.find(tname);
    	if (search == textures.end()) {
			std::string tname_full = basedir + tname;
			std::cout << "Load " << tname_full << std::endl;
			textures[tname] = new Surface(tname_full.c_str());
    	}
	}

	nr_objects = 0;
    for (size_t s = 0; s < shapes.size(); s++)
		nr_objects += shapes[s].mesh.indices.size() / 3;

	objects = new Primitive*[nr_objects];
	Primitive** current = objects;

	for (size_t s = 0; s < shapes.size(); s++)
	{
		for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
		{
			Triangle* tri = new Triangle();

			Triangle::FromTinyObj(tri, &attrib, &shapes[s].mesh, f, materials, textures);

			*current = tri;
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

	// load materials
	nr_materials = 4;
	materials = new Material *[nr_materials] {
		new Material(   0,   0, 0 ),	// Diffuse
		new Material( 0.3,   0, 0 ),	// Diffuse & reflective
		new Material( 0.2, 1.5, 0.15 ), // Glass
		new Material(   1,   0, 0 )		// Mirror
	};

	// load lights
	// All lights should have atleast one color value != 0
	nr_lights = 1;
	lights = new Light *[nr_lights] {
#ifdef USEPATHTRACE
		new SphereLight( vec3( 3, 3, 5 ), 1, Color( 50, 50, 50 ) )
#else
		new PointLight( vec3( 5, 10, 5 ), Color( 100, 100, 100 ) )
#endif
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

	for (uint i = 0; i < nr_objects; i++)
	{
		if(objects[i]->material == nullptr)
			objects[i]->material = materials[0];
	}
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
	for ( uint i = 0; i < nr_objects; i++ )
	{
		if (objects[i]->Occludes( r ))
			return true;
	}
	return false;
}

Primitive* Game::Intersect( Ray* r )
{
	Primitive* found = nullptr;

	for (uint i = 0; i < nr_objects; i++)
	{
		if(objects[i]->Intersect(r))
			found = objects[i];
	}

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
	for ( size_t i = 0; i < nr_lights; i++ )
	{
		#ifdef USEPATHTRACE
		// TODO: take size of light into account -> higher chance for lights that are closer or bigger
			i = RandomIndex(nr_lights);
		#endif
		// compute origin and direction of shadow ray
		shadowRay.direction = lights[i]->PointOnLight() - shadowRay.origin;
		shadowRay.t = shadowRay.direction.length();
		shadowRay.direction *= (1 / shadowRay.t);

		fac = dot( shadowRay.direction, normal );

		// This would mean no additional color so let's early out.
		// TODO: use epsilon?
		if (fac <= 0)
			#ifdef USEPATHTRACE
				return total;
			#else
				continue;
			#endif

		#ifdef USEPATHTRACE
			// TODO: Check the normal of the light and the shadowray direction, their dot should also be >0
			// fac_light = ???
			float fac_light = 1;

			if (fac_light <= 0)
				return total;
		#endif

		// find intersection of shadow ray, check if it is between the light and object
		if ( CheckOcclusion( &shadowRay ) )
			#ifdef USEPATHTRACE
				return total;
			#else
				continue;
			#endif

		// angle * distance attenuation * color
		total += (fac / ( shadowRay.t * shadowRay.t )) * lights[i]->color;

		#ifdef USEPATHTRACE
			// TODO: Find some way of getting the Scene.LIGHTAREA.
			float light_area = 1;
			return INVPI * nr_lights * fac_light * light_area * total;
		#endif
	}
	return total;
}

Color Game::RayTrace(Ray r, uint depth, Primitive* obj, vec3 interPoint, vec3 interNormal, float angle, bool backfacing)
{
	if (obj->material->IsNotRefractive()) {
		if (obj->material->IsFullMirror()) {
			// TODO: why does inverting the angle fix it?
			angle *= -1;
			r.Reflect(interPoint, interNormal, angle);
			r.Offset(1e-3);
			return Trace(r, depth + 1);
		}

		if (obj->material->IsFullDiffuse()) {
			Color ill = DirectIllumination( interPoint + r.CalculateOffset(-1e-3), interNormal );
			return ill * obj->ColorAt( interPoint );
		}

		float s = obj->material->speculative;
		Color ill = DirectIllumination( interPoint + r.CalculateOffset(-1e-3), interNormal );
		// TODO: why does inverting the angle fix it?
		angle *= -1;
		r.Reflect(interPoint, interNormal, angle);
		r.Offset( 1e-3 );
		Color reflected = Trace( r, depth + 1 );
		return s * reflected + ( 1 - s ) * ill * obj->ColorAt( interPoint );
	}

	// compute reflected ray and color
	Ray reflectRay = Ray( interPoint, r.direction );
	reflectRay.Reflect( interPoint, interNormal, angle );
	reflectRay.Offset( 1e-3 );

	// into glass or out
	// air = 1.0, glass = 1.5
	float n = backfacing ? obj->material->refractive : 1.0f / obj->material->refractive;
	float k = 1 - ( n * n * ( 1 - angle * angle ) );

	if ( k < 0 )
	{
		// Total Internal Reflection
		Color reflectCol = Trace( reflectRay, depth + 1 );
		return obj->ColorAt( interPoint ) * reflectCol;
	}

	// Calculate the refractive ray, and its color
	vec3 refractDir = n * r.direction + interNormal * ( n * angle - sqrtf( k ) );
	refractDir.normalize();
	Ray refractiveRay( interPoint, refractDir );
	refractiveRay.Offset( 1e-3 );
	Color refractCol = Trace( refractiveRay, depth + 1 );

	// Beer's law
	// check if refractive ray hits a backface
	Color beers = Color(1, 1, 1);
	if ( backfacing )
	{
		Color inversedColor = Color( 1.0f, 1.0f, 1.0f ) - obj->InternalColor();
		Color absorbance = inversedColor * obj->material->density * -r.t;
		beers = Color(	expf(absorbance.r),
						expf(absorbance.g),
						expf(absorbance.b));
	}
	// Schlicks approximation to determine the amount of reflection vs refraction
	float R0 = ( obj->material->refractive - 1 ) / ( obj->material->refractive + 1 );
	R0 = R0 * R0;
	float Fr = 1.0f - angle ;
	Fr = R0 + ( 1.0f - R0 ) * Fr * Fr * Fr * Fr * Fr;

	Color reflectCol = Trace( reflectRay, depth + 1 );
	return obj->ColorAt( interPoint ) * ( Fr * reflectCol + ( 1.0f - Fr ) * refractCol ) * beers;
}

Color Game::PathTrace(Ray r, uint depth, Primitive* obj, vec3 interPoint, vec3 interNormal, float angle, bool backfacing)
{
	bool reflect = false;
	bool refract = false;

	if (obj->material->HasRefract())
	{
		refract = RandomFloat() < obj->material->speculative;
	} else if (obj->material->HasReflect())
	{
		reflect = RandomFloat() < obj->material->speculative;
	}

	if (refract) {
		float n = backfacing ? obj->material->refractive : 1.0f / obj->material->refractive;
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
			return obj->ColorAt( interPoint ) * Trace( refractiveRay, depth + 1 );
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
		return obj->ColorAt( interPoint ) * reflectCol;
	}

	// Random bounce
	vec3 random_dir = RandomPointOnHemisphere( 1, interNormal );
	Ray newRay = Ray( interPoint, random_dir );

	// irradiance
	Color ei = Trace( newRay, depth + 1 ) * dot( interNormal, random_dir );
	Color BRDF = obj->ColorAt( interPoint ) * INVPI;
	return PI * 2.0f * BRDF * ei;
}

Color Game::Trace(Ray r, uint depth)
{
	if (depth > MAX_NR_ITERATIONS)
		return Color(0, 0, 0);

	Primitive* obj = Intersect( &r );

	#ifdef USEPATHTRACE
		Light* light = IntersectLights( &r );
		if ( light != nullptr )
			return light->color;
	#endif

	// No intersection point found
	if ( obj == nullptr )
	{
		if (sky == nullptr)
			return SKYDOME_DEFAULT_COLOR;
		else
			return sky->FindColor(r.direction);
	}

	// intersection point found
	vec3 interPoint = r.origin + r.t * r.direction;
	vec3 interNormal = obj->NormalAt( interPoint );
	float angle = -dot( r.direction, interNormal );
	bool backfacing = angle < 0.0f;
	if ( backfacing )
	{
		interNormal *= -1;
		angle *= -1;
	}

	#ifdef USEPATHTRACE
		return PathTrace(r, depth, obj, interPoint, interNormal, angle, backfacing);
	#else
		return RayTrace(r, depth, obj, interPoint, interNormal, angle, backfacing);
	#endif
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
float rand1 = Rand( 1 );
float rand2 = Rand( 1 );
float rand3 = Rand( 1 );
float rand4 = Rand( 1 );
float rand5 = Rand( 1 );
float rand6 = Rand( 1 );
float rand7 = Rand( 1 );
float rand8 = Rand( 1 );

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	printf("Game Tick\n");

	vec3 p0 = view->TopLeft();
	Pixel* buf = screen->GetBuffer();

	int dist_x_max = screen->GetWidth() / 2;
	int dist_y_max = screen->GetHeight() / 2;
	float dist_total_max = 1 / sqrtf(dist_x_max * dist_x_max + dist_y_max * dist_y_max);
		
	float max_distanceImage = sqrtf(pow(screen->GetWidth()/2, 2.0f) + pow(screen->GetHeight()/2, 2.0f));

	for (int y = 0; y < screen->GetHeight(); y++)
	for (int x = 0; x < screen->GetWidth(); x++)
	{
		#ifdef SSAA
			// 4 rays with random offsett, then compute average
			float u = ((float)x + rand5) / screen->GetWidth();
			float v = ((float)y + rand6) / screen->GetHeight();
			vec3 dir = p0 + u * view->right + v * view->down;
			dir.normalize();
			Ray r = Ray( view->position, dir );
			Color color1 = Trace( r, 0 );

			u = ((float)x + rand1) / screen->GetWidth();
			v = ((float)y + rand7) / screen->GetHeight();
			dir = p0 + u * view->right + v * view->down;
			dir.normalize();
			r = Ray( view->position, dir );
			Color color2 = Trace( r, 0 );

			u = ((float)x + rand2) / screen->GetWidth();
			v = ((float)y + rand3) / screen->GetHeight();
			dir = p0 + u * view->right + v * view->down;
			dir.normalize();
			r = Ray( view->position, dir );
			Color color3 = Trace( r, 0 );

			u = ((float)x + rand8) / screen->GetWidth();
			v = ((float)y + rand4) / screen->GetHeight();
			dir = p0 + u * view->right + v * view->down;
			dir.normalize();
			r = Ray( view->position, dir );
			Color color4 = Trace( r, 0 );
			Color color;
			color.r = ( color1.r + color2.r + color3.r + color4.r ) * 0.25;
			color.g = ( color1.g + color2.g + color3.g + color4.g ) * 0.25;
			color.b = ( color1.b + color2.b + color3.b + color4.b ) * 0.25;
		#else
			float u = (float)x / screen->GetWidth();
			float v = (float)y / screen->GetHeight();
			vec3 dir = p0 + u * view->right + v * view->down;
			dir.normalize();

			Ray r = Ray( view->position, dir );

			Color color = Trace( r, 0 );
		#endif
		color.GammaCorrect();
		color.ChromaticAbberation( { u, v } );
		color.Vignetting( ( x - screen->GetWidth() / 2 ), ( y - screen->GetHeight() / 2 ), dist_total_max );

		#ifdef USEPATHTRACE
			*buf = color.ToPixel(*buf, unmoved_frames);
		#else
			*buf = color.ToPixel();
		#endif

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
	#ifdef USEPATHTRACE
		unmoved_frames = 0;
		screen->Clear(0);
	#endif
}
