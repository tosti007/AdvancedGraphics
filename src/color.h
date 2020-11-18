// Template, UU version
// IGAD/NHTV/UU - Jacco Bikker - 2006-2019

#pragma once

#include "vectors.h"
#include "utils.h"

namespace AdvancedGraphics {

#define REDMASK	(0xff0000)
#define GREENMASK (0x00ff00)
#define BLUEMASK (0x0000ff)

typedef unsigned int Pixel; // unsigned int is assumed to be 32-bit, which seems a safe assumption.

inline Pixel AddBlend( Pixel a_Pixel1, Pixel a_Pixel2 )
{
	const unsigned int r = (a_Pixel1 & REDMASK) + (a_Pixel2 & REDMASK);
	const unsigned int g = (a_Pixel1 & GREENMASK) + (a_Pixel2 & GREENMASK);
	const unsigned int b = (a_Pixel1 & BLUEMASK) + (a_Pixel2 & BLUEMASK);
	const unsigned r1 = (r & REDMASK) | (REDMASK * (r >> 24));
	const unsigned g1 = (g & GREENMASK) | (GREENMASK * (g >> 16));
	const unsigned b1 = (b & BLUEMASK) | (BLUEMASK * (b >> 8));
	return (r1 + g1 + b1);
}

// subtractive blending
inline Pixel SubBlend( Pixel a_Pixel1, Pixel a_Pixel2 )
{
	int red = (a_Pixel1 & REDMASK) - (a_Pixel2 & REDMASK);
	int green = (a_Pixel1 & GREENMASK) - (a_Pixel2 & GREENMASK);
	int blue = (a_Pixel1 & BLUEMASK) - (a_Pixel2 & BLUEMASK);
	if (red < 0) red = 0;
	if (green < 0) green = 0;
	if (blue < 0) blue = 0;
	return (Pixel)(red + green + blue);
}

// Pixel scaling
inline Pixel ScalePixel( Pixel c, int s )
{
	const unsigned int rb = (((c & (REDMASK|BLUEMASK)) * s) >> 5) & (REDMASK|BLUEMASK);
	const unsigned int g = (((c & GREENMASK) * s) >> 5) & GREENMASK;
	return rb + g;
}

typedef vec3 Color;

inline Pixel ColorToPixel( Color c )
{
	int r = clamp( (int)( c.x * 255.0f ), 0, 255 );
	int g = clamp( (int)( c.y * 255.0f ), 0, 255 );
	int b = clamp( (int)( c.z * 255.0f ), 0, 255 );

	return ( r << 16 ) + ( g << 8 ) + (b << 0);
}

inline Color PixelToColor( Pixel p )
{
	float r = ((p & REDMASK) >> 16) / 255.0f;
	float g = ((p & GREENMASK) >> 8) / 255.0f;
	float b = ((p & BLUEMASK) >> 0) / 255.0f;
	return Color(r, g, b);
}


}; // namespace AdvancedGraphics
