// Template, UU version
// IGAD/NHTV/UU - Jacco Bikker - 2006-2019

#pragma once

#include "vectors.h"

namespace AdvancedGraphics {

#define REDMASK	(0xff0000)
#define GREENMASK (0x00ff00)
#define BLUEMASK (0x0000ff)

typedef unsigned int Pixel; // unsigned int is assumed to be 32-bit, which seems a safe assumption.

Pixel AddBlend( Pixel a_Pixel1, Pixel a_Pixel2 );
Pixel SubBlend( Pixel a_Pixel1, Pixel a_Pixel2 );
Pixel ScalePixel( Pixel c, int s );

struct Color {
	float r, g, b;

	inline Color() = default;
	inline Color(float cr, float cg, float cb) : r(cr), g(cg),	b(cb) {}
	Color( Pixel p );

	void GammaCorrect();
	void Vignetting( int dist_x, int dist_y, float dist_total_max );
	void ChromaticAbberation(vec2 uv);
	Pixel ToPixel();
	Pixel ToPixel(Pixel origional, uint weight);

	void operator *= ( const float& a ) { r *= a; g *= a; b *= a; }
	void operator *= ( const Color& a ) { r *= a.r; g *= a.g; b *= a.b; }
	void operator /= ( const Color& a ) { r /= a.r; g /= a.g; b /= a.b; }
	void operator += ( const Color& a ) { r += a.r; g += a.g; b += a.b; }
};

Color operator * ( const float& a, const Color& c );
Color operator * ( const Color& c, const float& a );
Color operator * ( const Color& a, const Color& b );
Color operator + ( const Color& a, const Color& b );
Color operator - ( const Color &a, const Color &b );

}; // namespace AdvancedGraphics
