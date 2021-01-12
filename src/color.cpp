// Template, UU version
// IGAD/NHTV/UU - Jacco Bikker - 2006-2019

#include "precomp.h"
#include "color.h"
#include "utils.h"

namespace AdvancedGraphics {

Pixel AddBlend( Pixel a_Pixel1, Pixel a_Pixel2 )
{
	const unsigned int r = (a_Pixel1 & REDMASK) + (a_Pixel2 & REDMASK);
	const unsigned int g = (a_Pixel1 & GREENMASK) + (a_Pixel2 & GREENMASK);
	const unsigned int b = (a_Pixel1 & BLUEMASK) + (a_Pixel2 & BLUEMASK);
	const unsigned r1 = (r & REDMASK) | (REDMASK * (r >> 24));
	const unsigned g1 = (g & GREENMASK) | (GREENMASK * (g >> 16));
	const unsigned b1 = (b & BLUEMASK) | (BLUEMASK * (b >> 8));
	return (r1 + g1 + b1);
}

Pixel SubBlend( Pixel a_Pixel1, Pixel a_Pixel2 )
{
	int red = (a_Pixel1 & REDMASK) - (a_Pixel2 & REDMASK);
	int green = (a_Pixel1 & GREENMASK) - (a_Pixel2 & GREENMASK);
	int blue = (a_Pixel1 & BLUEMASK) - (a_Pixel2 & BLUEMASK);
	if (red < 0) red = 0;
	if (green < 0) green = 0;
	if (blue < 0) blue = 0;
	return (Pixel)(red + green + blue);
}


Pixel ScalePixel( Pixel c, int s )
{
	const unsigned int rb = (((c & (REDMASK|BLUEMASK)) * s) >> 5) & (REDMASK|BLUEMASK);
	const unsigned int g = (((c & GREENMASK) * s) >> 5) & GREENMASK;
	return rb + g;
}

Color::Color( Pixel p ) : Color(
	((p & REDMASK) >> 16) / 255.0f,
	((p & GREENMASK) >> 8) / 255.0f,
	((p & BLUEMASK) >> 0) / 255.0f
) {}

void GammaCorrectFloat(float& v)
{
	// https://software.intel.com/content/www/us/en/develop/documentation/ipp-dev-reference/top/volume-2-image-processing/image-color-conversion/gamma-correction.html
	if (v <  0.018f)
		v *= 4.5f;
	else
		v = 1.099f * sqrtf(v) - 0.099f;
}

void Color::GammaCorrect()
{
	GammaCorrectFloat(r);
	GammaCorrectFloat(g);
	GammaCorrectFloat(b);
}

void Color::Vignetting( int dist_x, int dist_y, float dist_total_max )
{

	float dist_total = 1 - dist_total_max * sqrtf( dist_x * dist_x + dist_y * dist_y );
	r *= dist_total;
	g *= dist_total;
	b *= dist_total;
}

void Color::ChromaticAbberation( vec2 uv )
{
	float chromaticAbberation = 33.6f;
	// 1 / 512 (resolution)
	vec2 texel = { 0.001953125, 0.001953125 };

	vec2 coords = ( uv - 0.5 ) * 2.0;
	float coordDot = coords.dot( coords );

	vec2 precompute = coords * chromaticAbberation * coordDot;

	vec2 uvR = uv - texel * precompute;
	vec2 uvB = uv + texel * precompute;
}

Pixel Color::ToPixel()
{
	uint cr = clamp( (int)( r * 255.0f ), 0, 255 );
	uint cg = clamp( (int)( g * 255.0f ), 0, 255 );
	uint cb = clamp( (int)( b * 255.0f ), 0, 255 );
	return ( cr << 16 ) + ( cg << 8 ) + (cb << 0);
}

Pixel Color::ToPixel(uint weight)
{
	float fac = 1.0f / weight;
	uint cr = clamp( (int)( r * fac * 255.0f ), 0, 255 );
	uint cg = clamp( (int)( g * fac * 255.0f ), 0, 255 );
	uint cb = clamp( (int)( b * fac * 255.0f ), 0, 255 );
	return ( cr << 16 ) + ( cg << 8 ) + (cb << 0);
}

Color operator * ( const float& a, const Color& c ) { return Color( c.r * a, c.g * a, c.b * a ); }
Color operator * ( const Color& c, const float& a ) { return a * c; }
Color operator * ( const Color& a, const Color& b ) { return Color(a.r * b.r, a.g * b.g, a.b * b.b); }
Color operator + ( const Color& a, const Color& b ) { return Color(a.r + b.r, a.g + b.g, a.b + b.b); }
Color operator - ( const Color &a, const Color &b ) { return Color( a.r - b.r, a.g - b.g, a.b - b.b ); }

}; // namespace AdvancedGraphics
