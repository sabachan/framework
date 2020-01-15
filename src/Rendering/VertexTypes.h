#ifndef Rendering_VertexTypes_H
#define Rendering_VertexTypes_H

#include <Core/ArrayView.h>
#include <Math/Vector.h>

struct D3D11_INPUT_ELEMENT_DESC;

namespace sg {
namespace rendering {
//=============================================================================
#define VERTEX_DECL()                                                       \
    static ArrayView<D3D11_INPUT_ELEMENT_DESC const> InputEltDesc();
//=============================================================================
// When declaring a new vertex type, please follow the following layout and
// naming rules, and order delaration using same order:
// - Follow this semantic order:
//      Pos
//      Normal
//      Tex
//      Col
//      BlendIndices
//      BlendWeights
// - Order floats before ints, and ints in increasing sizes,
// - Order number of components in increasing order.
//=============================================================================
struct Vertex_Pos2f_Tex2f
{
    float2 pos;
    float2 tex;
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex2f_Col4f
{
    float2 pos;
    float2 tex;
    float4 col;
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex2f_2Col4f
{
    float2 pos;
    float2 tex;
    float4 col[2];
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex2f_2Col4f_Col4ub
{
    float2 pos;
    float2 tex;
    float4 col4f[2];
    ubyte4 col4ub;
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex2f_Col4ub
{
    float2 pos;
    float2 tex;
    ubyte4 col;
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex2f_2Col4ub
{
    float2 pos;
    float2 tex;
    ubyte4 col[2];
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex2f_3Col4ub
{
    float2 pos;
    float2 tex;
    ubyte4 col[3];
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex3f_2Col4f
{
    float2 pos;
    float3 tex;
    float4 col[2];
    VERTEX_DECL();
};
struct Vertex_Pos2f_Tex4f_2Col4f
{
    float2 pos;
    float4 tex;
    float4 col[2];
    VERTEX_DECL();
};
struct Vertex_Pos3f_Pos2f_Tex2f_Col4f_Col4ub
{
    float3 pos;
    float2 pos2f;
    float2 tex;
    float4 col;
    ubyte4 col4ub;
    VERTEX_DECL();
};
struct Vertex_Pos3f_Normal3f_Tex2f_Col4f
{
    float3 pos;
    float3 normal;
    float2 tex;
    float4 col;
    VERTEX_DECL();
};
struct Vertex_Pos3f_Normal3f_Tex2f_Col4f_BlendIndices2u16_BlendWeights2u16
{
    float3 pos;
    float3 normal;
    float2 tex;
    float4 col;
    u16vec2 blendIndices;
    u16vec2 blendWeights;
    VERTEX_DECL();
};
struct Vertex_Pos3f_2Normal3f_Tex2f_Col4f_BlendIndices2u16_BlendWeights2u16
{
    float3 pos;
    float3 normals[2];
    float2 tex;
    float4 col;
    u16vec2 blendIndices;
    u16vec2 blendWeights;
    VERTEX_DECL();
};
struct Vertex_Pos3f_Tex2f
{
    float3 pos;
    float2 tex;
    VERTEX_DECL();
};
struct Vertex_Pos3f_Tex2f_Col4f
{
    float3 pos;
    float2 tex;
    float4 col;
    VERTEX_DECL();
};
struct Vertex_Pos3f_Tex2f_2Col4f_Col4ub
{
    float3 pos;
    float2 tex;
    float4 col4f[2];
    ubyte4 col4ub;
    VERTEX_DECL();
};
struct Vertex_Pos3f_Tex2f_Col4ub
{
    float2 pos;
    float2 tex;
    ubyte4 col;
    VERTEX_DECL();
};
struct Vertex_Pos3f_Tex2f_2Col4ub
{
    float3 pos;
    float2 tex;
    ubyte4 col[2];
    VERTEX_DECL();
};
struct Vertex_Pos3f_Tex2f_3Col4ub
{
    float3 pos;
    float2 tex;
    ubyte4 col[3];
    VERTEX_DECL();
};
struct Vertex_Pos3f_Tex2f_4Col4ub
{
    float3 pos;
    float2 tex;
    ubyte4 col[4];
    VERTEX_DECL();
};

//=============================================================================
}
}

#endif
