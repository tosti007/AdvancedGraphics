#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"
#include "ray.h"
#include "light.h"

// .obj loader
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

void Game::InitDefaultScene()
{
	nr_objects = 3;
	objects = new Primitive*[nr_objects] {
		new Sphere(vec3(0, 0, 10), 3, 0xff0000, materials[0]),
		new Plane(vec3(0, 1, 0), 2, 0xffffff, materials[2]),
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

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init(int argc, char **argv)
{
	printf("Initializing Game\n");
	view = new Camera(vec3(0, 0, 0), vec3(0, 0, 1));

	// load materials
	nr_materials = 3;
	materials = new Material*[nr_materials] {
		new Material(0, 0, 0.5), // Diffuse
		new Material(0.2, 0.8, 0), // Glass
		new Material(1, 0, 0) // Mirror
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
			vec3 ill = DirectIllumination( interPoint, interNormal ) ;
			return ill * r.obj->color;
		}

		// TODO speculative combinations

		return PixelToColor(0xffff00);

	} else {
		// TODO add skydome
		return Color(0);
	}
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
	char buffer [32];
	sprintf (buffer, "Pos: %f %f %f", view->position.x, view->position.y, view->position.z);
	screen->Print(buffer, 2, 2, 0xffff00);

	sprintf (buffer, "Dir: %f %f %f", view->direction.x, view->direction.y, view->direction.z);
	screen->Print(buffer, 2, 9, 0xffff00);

	sprintf (buffer, "Rgt: %f %f %f", view->right.x, view->right.y, view->right.z);
	screen->Print(buffer, 2, 16, 0xffff00);

	sprintf (buffer, "Dwn: %f %f %f", view->down.x, view->down.y, view->down.z);
	screen->Print(buffer, 2, 23, 0xffff00);

	// Timer
	sprintf( buffer, "FPS: %f", 1 / deltaTime);
	screen->Print( buffer, 2, 30, 0xffff00 );
}
