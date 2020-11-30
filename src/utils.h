// Template, UU version
// IGAD/NHTV/UU - Jacco Bikker - 2006-2019

#pragma once

#include "vectors.h"

#define clamp(v,a,b) ((std::min)((b),(std::max)((v),(a))))

#define PI 3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f
#define INVPI 0.31830988618379067153777f

#ifdef _MSC_VER
#define ALIGN( x ) __declspec( align( x ) )
#define MALLOC64( x ) _aligned_malloc( x, 64 )
#define FREE64( x ) _aligned_free( x )
#else
#define ALIGN( x ) __attribute__( ( aligned( x ) ) )
#define MALLOC64( x ) aligned_alloc( 64, x )
#define FREE64( x ) free( x )
#define __inline __attribute__( ( __always_inline__ ) )
#endif

#define PREFETCH(x)			_mm_prefetch((const char*)(x),_MM_HINT_T0)
#define PREFETCH_ONCE(x)	_mm_prefetch((const char*)(x),_MM_HINT_NTA)
#define PREFETCH_WRITE(x)	_m_prefetchw((const char*)(x))
#define loadss(mem)			_mm_load_ss((const float*const)(mem))
#define broadcastps(ps)		_mm_shuffle_ps((ps),(ps), 0)
#define broadcastss(ss)		broadcastps(loadss((ss)))

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define   likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define   likely(expr) __builtin_expect((expr),true )
#define unlikely(expr) __builtin_expect((expr),false)
#endif

#define BADFLOAT(x) ((*(uint*)&x & 0x7f000000) == 0x7f000000)

// deterministic rng
static uint seed = 0x12345678;
inline uint RandomUInt() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; }
inline float RandomFloat() { return RandomUInt() * 2.3283064365387e-10f; }
inline float Rand( float range ) { return RandomFloat() * range; }
inline size_t RandomIndex( uint range ) { return RandomFloat() * (range - 1); }

namespace AdvancedGraphics {

inline void NotifyUser( const char *s )
{
#ifdef _WIN32
	HWND hApp = FindWindow( NULL, WINDOW_TITLE );
	MessageBox( hApp, s, "ERROR", MB_OK );
#else
	std::cout << "ERROR: " << s << std::endl;
#endif
	exit( 0 );
}

inline vec3 RandomPointOnSphere(float radius)
{
	// From: https://mathworld.wolfram.com/SpherePointPicking.html
	// Equation 12, 13, and 14
	float x0, x1, x2, x3;
	float x02, x12, x22, x32;
	float sum;
	do {
		x0 = RandomFloat() * 2 - 1;
		x1 = RandomFloat() * 2 - 1;
		x2 = RandomFloat() * 2 - 1;
		x3 = RandomFloat() * 2 - 1;
		x02 = x0 * x0;
		x12 = x1 * x1;
		x22 = x2 * x2;
		x32 = x3 * x3;
		sum = x02 + x12 + x22 + x32;
	} while( sum >= 1);
	return (radius / sum) * vec3(
		2 * (x1 * x3 + x0 * x2), 
		2 * (x2 * x3 + x0 * x1),
		x02 + x32 - x12 -x22
		);
}

inline vec3 RandomPointOnHemisphere(float radius, vec3 interNormal)
{
	vec3 point = RandomPointOnSphere(radius);
	float angle = dot(point, interNormal);
	if ( angle < 0.0f )
		point *= -1;
	return point;
}

}; // namespace AdvancedGraphics
