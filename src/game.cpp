#include <stdarg.h> // For variable number of arguments

#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"
#include "light.h"

// .obj loader
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#define INVPI 0.31830988618379067153777f
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
			tri->color = PixelToColor(0x00ff00);

			Triangle::FromTinyObj(tri, &attrib, &shapes[s].mesh, f);

			*current = tri;
			current++;
		}
	}
}
void Game::LoadSkyBox()
{
	printf( "Loading skydome data...\n");
	float3 *pixels = nullptr;
	FREE64( pixels ); // just in case we're reloading
	pixels = 0;
	char t[] = "assets/skybox.hdr", *p;
	if ( p = strstr( t, ".hdr" ) )
	{
		// attempt to load skydome from binary file
		memcpy( strstr( t, ".hdr" ), ".bin", 4 );
		std::ifstream f( t, std::ios::binary );
		if ( f )
		{
			printf( "Loading cached hdr data...\n" );
			f.read( (char *)&skyWidth, sizeof( skyWidth ) );
			f.read( (char *)&skyHeight, sizeof( skyHeight ) );
			pixels = (float3 *)MALLOC64( skyWidth * skyHeight * sizeof( float3 ) );
			f.read( (char *)pixels, sizeof( float3 ) * skyWidth * skyHeight );
		}
		else
			memcpy( strstr( t, ".bin" ), ".hdr", 4 );
	}
	else
	{
		printf( "Bad skydome filename.\n" );
		return;
	}
	// if no binary file
	if ( !pixels )
	{
		// load skydome from original .hdr file
		printf( "Loading original hdr data... " );
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType( t, 0 );
		if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( t );
		FIBITMAP *dib = FreeImage_Load( fif, t );
		if ( !dib ) return;
		skyWidth = FreeImage_GetWidth( dib );
		skyHeight = FreeImage_GetHeight( dib );
		pixels = (float3 *)MALLOC64( skyWidth * skyHeight * sizeof( float3 ) );
		// line by line
		for ( int y = 0; y < skyHeight; y++ )
		{
			memcpy( pixels + y * skyWidth, FreeImage_GetScanLine( dib, skyHeight - 1 - y ), skyWidth * sizeof( float3 ) );
		}
		FreeImage_Unload( dib );
		// save skydome to binary file, .hdr is slow to load
		memcpy( strstr( t, ".hdr" ), ".bin", 4 );
		std::ofstream f( t, std::ios::binary );
		f.write( (char *)&skyWidth, sizeof( skyWidth ) );
		f.write( (char *)&skyHeight, sizeof( skyHeight ) );
		f.write( (char *)pixels, sizeof( float3 ) * skyWidth * skyHeight );
	}

	skyPixels = pixels;
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
	LoadSkyBox();

	// load materials
	nr_materials = 4;
	materials = new Material*[nr_materials] {
		new Material( 0, 0, 0.5 ),	 // Diffuse
		new Material( 0.3, 0, 0.7 ), // Diffuse & reflective
		new Material( 0.2, 0.8, 0 ), // Glass
		new Material( 1, 0, 0 )		 // Mirror
	};

	// load lights
	nr_lights = 1;
	lights = new Light*[nr_lights] {
		new Light( vec3( 0, 0, 0 ), vec3( 10, 10, 10 ) ),
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
		vec3 rayDirection = lights[i]->position - interPoint;
		float lightDistance = rayDirection.length();
		rayDirection.normalize();

		float fac = dot( rayDirection, normal );
		vec3 rayOffset = 1e-3 * normal;
		if (fac < 0) {
			fac *= -1;
			rayOffset *= -1;
		}

		Ray shadowRay = Ray( interPoint + rayOffset, rayDirection );

		// find intersection of shadow ray, check if it is between the light and object
		// TODO Intersect function that returns at the first time an intersection is found
		if ( Intersect( &shadowRay ) && shadowRay.t < lightDistance )
			continue;

		// distance attenuation * angle * color
		total += (1 / ( lightDistance * lightDistance ) ) * fac * lights[i]->color;
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
			r.direction -= 2 * dot(r.direction, interNormal) * interNormal;
			r.origin = interPoint;
			return Trace(r, depth - 1);
		}

		if (m->IsFullDiffuse()) {
			vec3 ill = DirectIllumination( interPoint, interNormal );
			return ill * r.obj->color;
		}

		// TODO speculative combinations
		if (m->IsReflectiveDiffuse()) {
			float s = m->reflective;
			vec3 ill = DirectIllumination( interPoint, interNormal );
			r.direction -= 2 * dot( r.direction, interNormal ) * interNormal;
			r.origin = interPoint;
			Color reflected = Trace( r, depth - 1 );
			return s * reflected + ( 1 - s ) * ill;
		}
		return PixelToColor(0xffff00);

	} else {
		// find skydome pixel color
		float u = 1 + atan2f( r.direction.x, -r.direction.z ) * INVPI;
		float v = acosf( r.direction.y ) * INVPI;
		
		int xPixel = float( skyWidth ) * 0.5 * u;
		int yPixel = float( skyHeight ) * v;
		int pixelIdx = yPixel * skyWidth + xPixel;
		float3 skyColor = skyPixels[clamp( pixelIdx, 0, skyHeight * skyWidth )];
		return Color(skyColor.x, skyColor.y, skyColor.z);
		//return Color( 0 );
	}
}

void Game::Print(size_t buflen, uint yline, const char *fmt, ...) {
	char buf[buflen];
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

		*buf = ColorToPixel(color);
		buf++;
	}

	// Write debug output
	Print(32, 0, "Pos: %f %f %f", view->position.x, view->position.y, view->position.z);
	
	Print(32, 1, "Dir: %f %f %f", view->direction.x, view->direction.y, view->direction.z);
	
	Print(32, 2, "Rgt: %f %f %f", view->right.x, view->right.y, view->right.z);
	
	Print(32, 3, "Dwn: %f %f %f", view->down.x, view->down.y, view->down.z);
	
	Print(32, 4, "FPS: %f", 1 / deltaTime);
}
