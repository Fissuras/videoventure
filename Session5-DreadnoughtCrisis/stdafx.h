// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define GAME_API __declspec(dllimport)

#include "targetver.h"

// SIMD intrinsics
#include "xmmintrin.h"

// standard C library includes
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <malloc.h>

// STL includes
#include <vector>
#include <deque>
#include <algorithm>

// fast floating-point
#include "xs_Float.h"

// TinyXML includes
#include "tinyxml.h"

// Box2D includes
#include "Box2d/Box2D.h"

// FastDelegate includes
#include "FastDelegate.h"

// 2d math
#include "Vector2.h"
#include "Sphere2.h"
#include "Matrix2.h"
#include "Transform2.h"
#include "AlignedBox2.h"

// 3d math
#include "Vector3.h"

// vector 4
#include "Vector4.h"

// color
#include "Color4.h"

// utility includes
#include "Hash.h"
#include "Database.h"
#include "Input.h"
#include "Random.h"


// TO DO: move all these definitions to more appropriate locations


// debug function
extern int DebugPrint(const char *format, ...);


// GLOBAL VALUES (HACK)

// frame values
extern GAME_API float frame_time;
extern GAME_API float frame_turns;

// simulation values
extern GAME_API float sim_rate;
extern GAME_API float sim_step;
extern GAME_API unsigned int sim_turn;
extern GAME_API float sim_fraction;

// camera position
extern GAME_API Vector2 camerapos[2];



// UTILITY

// cast anything to anything
template <typename O, typename I> inline O Cast(I i)
{
	union { I i; O o; } cast;
	cast.i = i;
	return cast.o;
}

// fast reciprocal square root
inline float InvSqrt(float x)
{
	float xhalf = 0.5f*x;
	union { float f; int i; } floatint;
	floatint.f = x;	// get bits for floating value
	floatint.i = 0x5f375a86 - (floatint.i >> 1); // gives initial guess y0
	floatint.f *= (1.5f-xhalf*floatint.f*floatint.f); // Newton step, repeating increases accuracy
	return floatint.f;
}

// linear interpolation
template<typename T> inline const T Lerp(T v0, T v1, float s)
{
	return (1 - s) * v0 + s * v1;
}

// value clamp
template<typename T> inline const T Clamp(T v, T min, T max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

// rectangle template
template<typename T> struct Rect
{
	T x;
	T y;
	T w;
	T h;
};


#ifndef SDL_arraysize
#define SDL_arraysize(array)	(sizeof(array)/sizeof(array[0]))
#endif


// CONFIGURATION

#define USE_POOL_ALLOCATOR

#ifdef USE_POOL_ALLOCATOR
#include <boost/pool/pool.hpp>
#endif

namespace std
{
	struct RTTI_NOT_SUPPORTED;
	typedef RTTI_NOT_SUPPORTED type_info;
}
#define typeid *( ::std::type_info* )sizeof 
#include <boost/variant.hpp>