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
	nr_objects = 3;
	objects = new Primitive*[nr_objects] {
		new Sphere(vec3(0, 0, 10), 3, 0xff0000, materials[0]),
		new Plane(vec3(0, 1, 0), 2, 0xffffff, materials[1]),
		new Triangle(vec3(0, 0, 15), vec3(4, 5, 12), vec3(6, -6, 13), 0x0000ff, materials[0])
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
void Game::InitSkyBox()
{
	printf( "Loading skydome data...\n");
	if (skyPixels != nullptr)
		FREE64( skyPixels ); // just in case we're reloading
	skyPixels = nullptr;
	char filename[] = "assets/skybox.hdr";
	
	char* ext = strstr( filename, ".hdr" );
	if ( ext != nullptr )
	{
		// Let's try the binary file first.
		memcpy( ext, ".bin", 4 );
	}

	ext = strstr( filename, ".bin" );
	if ( ext == nullptr )
	{
		std::cerr << "Skydome filename should either be *.hdr or *.bin" << std::endl;
		exit(1);
	}

	// attempt to load skydome from binary file
	std::ifstream f_bin( filename, std::ios::binary );
	if ( f_bin.good() )
	{
		// A correct binary file
		printf( "Loading cached hdr data...\n" );
		f_bin.read( (char *)&skyWidth, sizeof( skyWidth ) );
		f_bin.read( (char *)&skyHeight, sizeof( skyHeight ) );
		skyPixels = (Color*)MALLOC64( skyWidth * skyHeight * sizeof(Color) );
		f_bin.read( (char *)skyPixels, sizeof( Color ) * skyWidth * skyHeight );
	}
	else
	{
		// Not a correct binary file, so let's use the HDR
		memcpy( ext, ".hdr", 4 );

		// load skydome from original .hdr file
		printf( "Loading original hdr data...\n" );
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType( filename, 0 );
		if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( filename );
		FIBITMAP *dib = FreeImage_Load( fif, filename );
		if ( !dib ) {
			std::cerr << "Could not load image!" << std::endl;
			exit(1);
		}

		skyWidth = FreeImage_GetWidth( dib );
		skyHeight = FreeImage_GetHeight( dib );
		skyPixels = (Color *)MALLOC64( skyWidth * skyHeight * sizeof(Color) );
		// line by line
		for ( uint y = 0; y < skyHeight; y++ )
		{
			memcpy( skyPixels + y * skyWidth, FreeImage_GetScanLine( dib, skyHeight - 1 - y ), skyWidth * sizeof(Color) );
		}
		FreeImage_Unload( dib );

		// save skydome to binary file, .hdr is slow to load
		memcpy( ext, ".bin", 4 );
		std::ofstream f_hdr( filename, std::ios::binary );
		f_hdr.write( (char *)&skyWidth, sizeof( skyWidth ) );
		f_hdr.write( (char *)&skyHeight, sizeof( skyHeight ) );
		f_hdr.write( (char *)skyPixels, sizeof(Color) * skyWidth * skyHeight );
		f_hdr.close();
	}
	f_bin.close();
}

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init(int argc, char **argv)
{
	printf("Initializing Game\n");
	view = new Camera(vec3(0, 0, 0), vec3(0, 0, 1));

	// load skybox
	skyPixels = 0;
	InitSkyBox();

	// load materials
	nr_materials = 4;
	materials = new Material*[nr_materials] {
		new Material( 0, 0, 0.5 ),	 // Diffuse
		new Material( 0.3, 0, 0.7 ), // Diffuse & reflective
		new Material( 0.2, 0.8, 0 ), // Glass
		new Material( 1, 0, 0 )		 // Mirror
	};

	// load lights
	// All lights should have atleast one color value != 0
	nr_lights = 1;
	lights = new Light*[nr_lights] {
		new Light( vec3( 0, 0, 0 ), Color( 10, 10, 10 ) ),
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

bool Game::Intersect( Ray* r )
{
	bool found = false;
	// TODO: replace this with an iteration over all primitives.

	for (uint i = 0; i < nr_objects; i++)
	{
		found |= objects[i]->Intersect(r);
	}

	return found;
}

Color Game::DirectIllumination( vec3 interPoint, vec3 normal )
{
	// accumulated color
	Color total( 0 );
	// send shadow ray to each light and add its color
	for ( size_t i = 0; i < nr_lights; i++ )
	{
		// compute origin and direction of shadow ray
		Ray shadowRay = Ray( interPoint, lights[i]->position - interPoint );
		shadowRay.t = shadowRay.direction.length();
		shadowRay.direction *= (1 / shadowRay.t);

		float fac = dot( shadowRay.direction, normal );

		// This would mean no additional color so let's early out.
		// TODO: use epsilon?
		if (fac == 0)
			continue;

		vec3 rayOffset = 1e-3 * normal;
		if (fac < 0) {
			fac *= -1;
			rayOffset *= -1;
		}

		shadowRay.origin += rayOffset;

		// find intersection of shadow ray, check if it is between the light and object
		if ( CheckOcclusion( &shadowRay ) )
			continue;

		// distance attenuation * angle * color
		total += (1 / ( shadowRay.t * shadowRay.t ) ) * fac * lights[i]->color;
	}
	return total;
}

Color Game::Trace(Ray r, uint depth)
{
	// TODO: handle depth value.

	if ( Intersect( &r ) )
	{
		// intersection point
		vec3 interPoint = r.origin + r.t * r.direction;
		vec3 interNormal = r.obj->NormalAt( interPoint );
		
		Material* m = materials[0]; // Default diffuse
		if (r.obj->material != NULL)
			m = r.obj->material;

		if (m->IsFullMirror()) {
			r.Reflect(interPoint, interNormal);
			return Trace(r, depth - 1);
		}

		if (m->IsFullDiffuse()) {
			Color ill = DirectIllumination( interPoint, interNormal );
			return ill * r.obj->color;
		}

		// TODO speculative combinations
		if (m->IsReflectiveDiffuse()) {
			float s = m->reflective;
			Color ill = DirectIllumination( interPoint, interNormal );
			r.Reflect(interPoint, interNormal);
			Color reflected = Trace( r, depth - 1 );
			return s * reflected + ( 1 - s ) * ill;
		}
		return Color(0xffff00);

	} else {
		// find skydome pixel color
		float u = 1 + atan2f( r.direction.x, -r.direction.z ) * INVPI;
		float v = acosf( r.direction.y ) * INVPI;
		
		uint xPixel = float( skyWidth ) * 0.5 * u;
		uint yPixel = float( skyHeight ) * v;
		uint pixelIdx = yPixel * skyWidth + xPixel;
		return skyPixels[std::min( pixelIdx, skyHeight * skyWidth )];
	}
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

		Color color = Trace( r, 1 );

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
