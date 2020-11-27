#include <stdarg.h> // For variable number of arguments

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
		new Sphere( vec3( -3, 2, 10 ), 2.5, 0xffffff, materials[1] ),
		new Sphere( vec3( 3, 2, 10 ), 2.5, 0xffffff, materials[1] ),
		new Sphere( vec3( -2, 0, 5 ), 0.75, 0xff0000, materials[2] ),
		new Sphere( vec3( 0, 0, 5 ), 0.75, 0x00ff00, materials[2] ),
		new Sphere( vec3( 2, 0, 5 ), 0.75, 0x0000ff, materials[2] ),
		new Plane( vec3( 0, 1, 0 ), 2, 0xff8833, materials[0] ),
		new Triangle( vec3( 0, 0, 15 ), vec3( 4, 5, 12 ), vec3( 6, -6, 13 ), 0x0000ff, materials[1] )
	};
}

void Game::InitFromTinyObj( char* filename )
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, filename );
	if ( !warn.empty() )
		std::cout << warn << std::endl;
	if ( !err.empty() )
		std::cerr << err << std::endl;
	if ( !ret )
		exit( 1 );

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
			tri->color = Color(0x00ff00);

			Triangle::FromTinyObj(tri, &attrib, &shapes[s].mesh, f);

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
		new Material(   0,   0 ), // Diffuse
		new Material( 0.9,   0 ), // Diffuse & reflective
		new Material( 0.2, 0.8 ), // Glass
		new Material(   1,   0 )  // Mirror
	};

	// load lights
	// All lights should have atleast one color value != 0
	nr_lights = 1;
	lights = new Light *[nr_lights] {
		new Light( vec3( 0, 10, 5 ), Color( 100, 100, 20 ) )
	};

	// load model
	switch (argc)
	{
		case 1: // No arguments
			InitDefaultScene();
			break;
		case 2: // An obj file
			InitFromTinyObj(argv[1]);
			break;
		default:
			std::cout << argc << " arguments not accepted!" << std::endl;
			exit(1);
			break;
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

Color Game::DirectIllumination( vec3 interPoint, vec3 normal )
{
	// accumulated color
	Color total( 0 );
	Ray shadowRay(vec3(0), vec3(0));
	float fac;
	// send shadow ray to each light and add its color
	for ( size_t i = 0; i < nr_lights; i++ )
	{
		// compute origin and direction of shadow ray
		shadowRay.origin = interPoint;
		shadowRay.direction = lights[i]->position - interPoint;
		shadowRay.t = shadowRay.direction.length();
		shadowRay.direction *= (1 / shadowRay.t);

		fac = dot( shadowRay.direction, normal );

		// This would mean no additional color so let's early out.
		// TODO: use epsilon?
		if (fac == 0)
			continue;

		if (fac < 0) {
			fac *= -1;
		}

		shadowRay.Offset(1e-3);

		// find intersection of shadow ray, check if it is between the light and object
		if ( CheckOcclusion( &shadowRay ) )
			continue;

		// angle * distance attenuation * color
		total += (fac / ( shadowRay.t * shadowRay.t )) * lights[i]->color;
	}
	return total;
}

Color Game::Trace(Ray r, uint depth)
{
	// TODO: handle depth value.
	Primitive* obj = Intersect( &r );

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
	
	Material* m = materials[0]; // Default diffuse
	if (obj->material != nullptr)
		m = obj->material;

	if (m->IsFullMirror()) {
		r.Reflect(interPoint, interNormal);
		r.Offset(1e-3);
		return Trace(r, depth - 1);
	}

	if (m->IsFullDiffuse()) {
		Color ill = DirectIllumination( interPoint + r.CalculateOffset(-1e-3), interNormal );
		return ill * obj->color;
	}

	if (m->IsNotRefractive()) {
		float s = m->speculative;
		Color ill = DirectIllumination( interPoint + r.CalculateOffset(-1e-3), interNormal );
		r.Reflect(interPoint, interNormal);
		r.Offset( 1e-3 );
		Color reflected = Trace( r, depth - 1 );
		return s * reflected + ( 1 - s ) * ill;
	}

	bool backFacing = dot( r.direction, interNormal ) > 0.0f;
	if ( backFacing )
		interNormal *= -1;
	
	// compute reflected ray and color
	Ray reflectRay = Ray( interPoint, r.direction );
	reflectRay.Reflect( interPoint, interNormal );
	reflectRay.Offset( 1e-3 );

	// into glass or out
	bool entering = !backFacing;
	// air = 1.0, glass = 1.5
	float n1 = entering ? 1.0f : 1.5f;
	float n2 = entering ? 1.5f : 1.0f;
	float n = n1 / n2;

	// Angle of ray with normal
	float cosI = dot( interNormal, -r.direction );
	float k = 1 - ( n * n * ( 1 - cosI * cosI ) );

	if ( k < 0 )
	{
		// Total Internal Reflection
		Color reflectCol = Trace( reflectRay, depth - 1 );
		return obj->color * reflectCol;
	}

	// Calculate the refractive ray, and its color
	vec3 refractDir = n * r.direction + interNormal * ( n * cosI - sqrtf( k ) );
	Ray refractiveRay( interPoint, normalize( refractDir ) );
	refractiveRay.Offset( 1e-3 );
	Color refractCol = Trace( refractiveRay, depth - 1 );

	// Schlicks approximation to determine the amount of reflection vs refraction
	float R0 = powf( ( n1 - n2 ) / ( n1 + n2 ), 2.0f );
	float Fr = R0 + ( 1.0f - R0 ) * powf( ( 1.0f - cosI ), 5.0f );

	Color reflectCol = Trace( reflectRay, depth - 1 );
	return obj->color * ( Fr * reflectCol + ( 1.0f - Fr ) * refractCol );
}

void Game::Print(size_t buflen, uint yline, const char *fmt, ...) {
	char buf[128];
	va_list va;
    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);
	screen->Print(buf, 2, 2 + yline * 7, 0xffff00);
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	printf("Game Tick\n");

	vec3 p0 = view->TopLeft();
	Pixel* buf = screen->GetBuffer();

	for (int y = 0; y < screen->GetHeight(); y++)
	for (int x = 0; x < screen->GetWidth(); x++)
	{
		float u = (float)x / screen->GetWidth();
		float v = (float)y / screen->GetHeight();
		vec3 dir = p0 + u * view->right + v * view->down;
		dir.normalize();

		Ray r = Ray(view->position, dir);

		Color color = Trace( r, 4 );

		color.GammaCorrect();
		*buf = color.ToPixel();
		buf++;
	}

	// Write debug output
	Print(32, 0, "Pos: %f %f %f", view->position.x, view->position.y, view->position.z);
	
	Print(32, 1, "Dir: %f %f %f", view->direction.x, view->direction.y, view->direction.z);
	
	Print(32, 2, "Rgt: %f %f %f", view->right.x, view->right.y, view->right.z);
	
	Print(32, 3, "Dwn: %f %f %f", view->down.x, view->down.y, view->down.z);
	
	Print(32, 4, "FPS: %f", 1 / deltaTime);
}
