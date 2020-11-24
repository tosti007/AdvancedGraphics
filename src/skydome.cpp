#include "precomp.h" // include (only) this in every .cpp file
#include "skydome.h"
#include "utils.h"

SkyDome::SkyDome() : width(0), height(0), pixels(nullptr)
{
	printf( "Loading skydome data...\n");
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
		f_bin.read( (char *)&width, sizeof( width ) );
		f_bin.read( (char *)&height, sizeof( height ) );
		pixels = (Color*)MALLOC64( width * height * sizeof(Color) );
		f_bin.read( (char *)pixels, sizeof( Color ) * width * height );
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

		width = FreeImage_GetWidth( dib );
		height = FreeImage_GetHeight( dib );
		pixels = (Color *)MALLOC64( width * height * sizeof(Color) );
		// line by line
		for ( uint y = 0; y < height; y++ )
		{
			memcpy( pixels + y * width, FreeImage_GetScanLine( dib, height - 1 - y ), width * sizeof(Color) );
		}
		FreeImage_Unload( dib );

		// save skydome to binary file, .hdr is slow to load
		memcpy( ext, ".bin", 4 );
		std::ofstream f_hdr( filename, std::ios::binary );
		f_hdr.write( (char *)&width, sizeof( width ) );
		f_hdr.write( (char *)&height, sizeof( height ) );
		f_hdr.write( (char *)pixels, sizeof(Color) * width * height );
		f_hdr.close();
	}
	f_bin.close();
}

Color SkyDome::FindColor(vec3 direction)
{
    float u = 1 + atan2f( direction.x, -direction.z ) * INVPI;
    float v = acosf( direction.y ) * INVPI;
    uint x = width / 2 * u;
    uint y = height * v;
    uint idx = y * width + x;
    if (idx >= height * width)
        return SKYDOME_DEFAULT_COLOR;
    return pixels[idx];
}