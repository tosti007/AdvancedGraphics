// Template, UU version
// IGAD/NHTV/UU - Jacco Bikker - 2006-2019

#pragma once

#define WINDOW_TITLE	"Advanced Graphics"
#define SCRWIDTH 1280
#define SCRHEIGHT 720

typedef unsigned char uchar;
typedef unsigned char byte;
typedef int64_t int64;
typedef uint64_t uint64;
typedef unsigned int uint;

#define clamp(v,a,b) ((std::min)((b),(std::max)((v),(a))))

#define PI					3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f

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

struct timer
{
	typedef std::chrono::high_resolution_clock Clock;
	typedef Clock::time_point TimePoint;
	typedef std::chrono::microseconds MicroSeconds;

	TimePoint start;
	inline timer() : start( get() ) {}

	/// Returns the elapsed time, in milliseconds.
	inline float elapsed() const
	{
		auto diff = get() - start;
		auto duration_us = std::chrono::duration_cast<MicroSeconds>( diff );
		return static_cast<float>( duration_us.count() ) / 1000.0f;
	}
	static inline TimePoint get()
	{
		return Clock::now();
	}

	inline void reset() { start = get(); }
};

}; // namespace AdvancedGraphics
