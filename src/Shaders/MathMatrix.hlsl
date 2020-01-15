#ifndef MathMatrix4x4_HLSL
#define MathMatrix4x4_HLSL

//=============================================================================
struct math_Matrix4x4
{
    float4 rows[4];
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix4x4 transpose(math_Matrix4x4 m)
{
    math_Matrix4x4 r;
    r.rows[0] = float4(m.rows[0].x, m.rows[1].x, m.rows[2].x, m.rows[3].x);
    r.rows[1] = float4(m.rows[0].y, m.rows[1].y, m.rows[2].y, m.rows[3].y);
    r.rows[2] = float4(m.rows[0].z, m.rows[1].z, m.rows[2].z, m.rows[3].z);
    r.rows[3] = float4(m.rows[0].w, m.rows[1].w, m.rows[2].w, m.rows[3].w);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix4x4 mul(math_Matrix4x4 ma, math_Matrix4x4 mb)
{
    math_Matrix4x4 r;
    math_Matrix4x4 mc = transpose(mb);
    r.rows[0] = float4( dot(ma.rows[0], mc.rows[0]), dot(ma.rows[0], mc.rows[1]), dot(ma.rows[0], mc.rows[2]), dot(ma.rows[0], mc.rows[3]) );
    r.rows[1] = float4( dot(ma.rows[1], mc.rows[0]), dot(ma.rows[1], mc.rows[1]), dot(ma.rows[1], mc.rows[2]), dot(ma.rows[1], mc.rows[3]) );
    r.rows[2] = float4( dot(ma.rows[2], mc.rows[0]), dot(ma.rows[2], mc.rows[1]), dot(ma.rows[2], mc.rows[2]), dot(ma.rows[2], mc.rows[3]) );
    r.rows[3] = float4( dot(ma.rows[3], mc.rows[0]), dot(ma.rows[3], mc.rows[1]), dot(ma.rows[3], mc.rows[2]), dot(ma.rows[3], mc.rows[3]) );
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float4 mul(math_Matrix4x4 m, float4 v)
{
    float4 r;
    r = float4(dot(m.rows[0], v), dot(m.rows[1], v), dot(m.rows[2], v), dot(m.rows[3], v));
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix4x4 math_CreateMatrix4x4()
{
    math_Matrix4x4 r;
    r.rows[0] = r.rows[1] = r.rows[2] = r.rows[3] = float4(0,0,0,0);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix4x4 math_CreateMatrix4x4(float4 rows[4])
{
    math_Matrix4x4 r = math_CreateMatrix4x4();
    r.rows[0] = rows[0]; r.rows[1] = rows[1]; r.rows[2] = rows[2]; r.rows[3] = rows[3];
    return r;
}
//=============================================================================
// NB: The disambiguaterows/cols are used to workaround the dumbest design in
// hlsl: function overloading is based on struct underlying layout ONLY.
// Hence, 2 struct containing the same types are equal!
struct math_Matrix2x2 { math_Matrix4x4 m; int2 disambiguaterows; float2 disambiguatecols; };
struct math_Matrix2x3 { math_Matrix4x4 m; int2 disambiguaterows; float2 disambiguatecols; };
struct math_Matrix2x4 { math_Matrix4x4 m; int2 disambiguaterows; float2 disambiguatecols; };
struct math_Matrix3x2 { math_Matrix4x4 m; int3 disambiguaterows; float3 disambiguatecols; };
struct math_Matrix3x3 { math_Matrix4x4 m; int3 disambiguaterows; float3 disambiguatecols; };
struct math_Matrix3x4 { math_Matrix4x4 m; int3 disambiguaterows; float3 disambiguatecols; };
struct math_Matrix4x2 { math_Matrix4x4 m; int4 disambiguaterows; float4 disambiguatecols; };
struct math_Matrix4x3 { math_Matrix4x4 m; int4 disambiguaterows; float4 disambiguatecols; };
//=============================================================================
math_Matrix2x2 math_CreateMatrix2x2(float2 rows[2])
{
    math_Matrix2x2 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = float4(rows[0], 0, 0); r.m.rows[1] = float4(rows[1], 0, 0);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix3x2 math_CreateMatrix3x2(float2 rows[3])
{
    math_Matrix3x2 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = float4(rows[0], 0, 0); r.m.rows[1] = float4(rows[1], 0, 0); r.m.rows[2] = float4(rows[2], 0, 0);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix4x2 math_CreateMatrix4x2(float2 rows[4])
{
    math_Matrix4x2 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = float4(rows[0], 0, 0); r.m.rows[1] = float4(rows[1], 0, 0); r.m.rows[2] = float4(rows[2], 0, 0); r.m.rows[3] = float4(rows[3], 0, 0);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix2x3 math_CreateMatrix2x3(float3 rows[2])
{
    math_Matrix2x3 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = float4(rows[0], 0); r.m.rows[1] = float4(rows[1], 0);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix3x3 math_CreateMatrix3x3(float3 rows[3])
{
    math_Matrix3x3 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = float4(rows[0], 0); r.m.rows[1] = float4(rows[1], 0); r.m.rows[2] = float4(rows[2], 0);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix4x3 math_CreateMatrix4x3(float3 rows[4])
{
    math_Matrix4x3 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = float4(rows[0], 0); r.m.rows[1] = float4(rows[1], 0); r.m.rows[2] = float4(rows[2], 0); r.m.rows[3] = float4(rows[3], 0);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix2x4 math_CreateMatrix2x4(float4 rows[2])
{
    math_Matrix2x4 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = rows[0]; r.m.rows[1] = rows[1];
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
math_Matrix3x4 math_CreateMatrix3x4(float4 rows0, float4 rows1, float4 rows2)
{
    math_Matrix3x4 r;
    r.m = math_CreateMatrix4x4();
    r.m.rows[0] = rows0; r.m.rows[1] = rows1; r.m.rows[2] = rows2;
    return r;
}
math_Matrix3x4 math_CreateMatrix3x4(float4 rows[3]) { return math_CreateMatrix3x4(rows[0], rows[1], rows[2]); }
//=============================================================================
float2 mul(math_Matrix2x2 m, float2 v) { return mul(m.m, float4(v,0,0)).xy ; }
float3 mul(math_Matrix3x2 m, float2 v) { return mul(m.m, float4(v,0,0)).xyz; }
float4 mul(math_Matrix4x2 m, float2 v) { return mul(m.m, float4(v,0,0))    ; }
float2 mul(math_Matrix2x3 m, float3 v) { return mul(m.m, float4(v,  0)).xy ; }
float3 mul(math_Matrix3x3 m, float3 v) { return mul(m.m, float4(v,  0)).xyz; }
float4 mul(math_Matrix4x3 m, float3 v) { return mul(m.m, float4(v,  0))    ; }
float2 mul(math_Matrix2x4 m, float4 v) { return mul(m.m,        v     ).xy ; }
float3 mul(math_Matrix3x4 m, float4 v) { return mul(m.m,        v     ).xyz; }
//=============================================================================

#endif
