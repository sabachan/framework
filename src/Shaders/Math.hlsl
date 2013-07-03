#ifndef Math_HLSL
#define Math_HLSL

//=============================================================================
float math_sq(float v) { return v*v; }
float math_det(float2 u, float2 v) { return u.x*v.y - u.y*v.x; }
//=============================================================================

#endif
