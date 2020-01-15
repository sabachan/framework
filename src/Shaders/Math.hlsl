#ifndef Math_HLSL
#define Math_HLSL

#include "MathMatrix.hlsl"

#define math_TAU (6.283185307179586476925)
#define math_PI  (3.14159265358979323846)
//=============================================================================
float math_sq(float v) { return v*v; }
float math_det(float2 u, float2 v) { return u.x*v.y - u.y*v.x; }
//=============================================================================

#endif
