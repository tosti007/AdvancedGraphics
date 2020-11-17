// Template, UU version
// IGAD/NHTV/UU - Jacco Bikker - 2006-2019

#pragma once

namespace AdvancedGraphics {

#define REDMASK	(0xff0000)
#define GREENMASK (0x00ff00)
#define BLUEMASK (0x0000ff)

typedef unsigned int Color; // unsigned int is assumed to be 32-bit, which seems a safe assumption.

inline Color AddBlend( Color a_Color1, Color a_Color2 )
{
	const unsigned int r = (a_Color1 & REDMASK) + (a_Color2 & REDMASK);
	const unsigned int g = (a_Color1 & GREENMASK) + (a_Color2 & GREENMASK);
	const unsigned int b = (a_Color1 & BLUEMASK) + (a_Color2 & BLUEMASK);
	const unsigned r1 = (r & REDMASK) | (REDMASK * (r >> 24));
	const unsigned g1 = (g & GREENMASK) | (GREENMASK * (g >> 16));
	const unsigned b1 = (b & BLUEMASK) | (BLUEMASK * (b >> 8));
	return (r1 + g1 + b1);
}

// subtractive blending
inline Color SubBlend( Color a_Color1, Color a_Color2 )
{
	int red = (a_Color1 & REDMASK) - (a_Color2 & REDMASK);
	int green = (a_Color1 & GREENMASK) - (a_Color2 & GREENMASK);
	int blue = (a_Color1 & BLUEMASK) - (a_Color2 & BLUEMASK);
	if (red < 0) red = 0;
	if (green < 0) green = 0;
	if (blue < 0) blue = 0;
	return (Color)(red + green + blue);
}

// color scaling
inline Color ScaleColor( Color c, int s )
{
	const unsigned int rb = (((c & (REDMASK|BLUEMASK)) * s) >> 5) & (REDMASK|BLUEMASK);
	const unsigned int g = (((c & GREENMASK) * s) >> 5) & GREENMASK;
	return rb + g;
}

}; // namespace AdvancedGraphics
