// add your includes to this file instead of to individual .cpp files
// to enjoy the benefits of precompiled headers:
// - fast compilation
// - solve issues with the order of header files once (here)
// do not include headers in header files (ever).

// Prevent expansion clashes (when using std::min and std::max):
#define NOMINMAX

// #define FULLSCREEN
// #define ADVANCEDGL	// faster if your system supports it

// Glew should be included first
#include <GL/glew.h>
// Comment for autoformatters: prevent reordering these two.
#include <GL/gl.h>

#ifdef _WIN32
// Followed by the Windows header
#include <Windows.h>

// Then import wglext: This library tries to include the Windows
// header WIN32_LEAN_AND_MEAN, unless it was already imported.
#include <GL/wglext.h>

// Extra definitions for redirectIO
#include <fcntl.h>
#include <io.h>
#endif

// External dependencies:
#include <FreeImage.h>
#include <SDL.h>

// C++ headers
#include <fstream>
#include <iostream>
#include <memory>

// Namespaced C headers:
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Header for AVX, and every technology before it.
// If your CPU does not support this, include the appropriate header instead.
// See: https://stackoverflow.com/a/11228864/2844473
#include <immintrin.h>

#define WINDOW_TITLE "Advanced Graphics"
#define SCRWIDTH 512
#define SCRHEIGHT 512

#define DEFAULT_OBJECT_COLOR Color(1, 0, 0)
#define MAX_NR_ITERATIONS 4
#define NR_LIGHT_SAMPLES 1

// Anti-aliasing 4x
//#define SSAA
//#define USESTRATIFICATION
#define USENEE
#define USERUSSIANROULETTE
//#define USEMIS
//#define USEVIGNETTING
#define USEBVH
//#define VISUALIZEBVH
// Number of bins to use for BVH
//  - For full SAH use 0
//  - For median split use 2
//  - For any other amount of bins use n
#define BVHBINS 8

// Kernel size for filtering
// If this is 0 then no filter is applied.
#define KERNEL_SIZE 65
#define SIGMA_ILLUMINATION 50.0f
#define SIGMA_FIREFLY 25.0f
//#define OPENCV2

typedef unsigned char uchar;
typedef unsigned char byte;
typedef int64_t int64;
typedef uint64_t uint64;
typedef unsigned int uint;

// Leak everything so we can easilly use it. 
// This can be considered bad practice,
// but since we are using only one namespace it can't hurt too much.
namespace AdvancedGraphics {};
using namespace AdvancedGraphics;
