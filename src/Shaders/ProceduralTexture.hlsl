#ifndef ProceduralTexture_HLSL
#define ProceduralTexture_HLSL

#include "Random.hlsl"
#include "Perlin.hlsl"
#include "CirclesFont.hlsl"

uniform uint date_in_frames;

static const float PI = 3.14159265359f;

//=============================================================================
struct ptex_Simplex2D
{
    uint2 id;
    uint2 verticesId[3];
    float2 vertices[3];
    float barycentricCoordinates[3];
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ptex_Simplex2D ptex_GetSimplex(in float2 pos)
{
    ptex_Simplex2D simplex;
    float2 Ex = float2(1,0);
    float2 Ey = float2(-cos(PI/3.f), sin(PI/3.f));
    float2 Ei = float2(cos(PI/3.f), sin(PI/3.f)); // 3rd edge, viewed as the "internal" diagonal of the skewed grid.
    float2 Nx = float2(0,1);
    float2 Ny = float2(Ey.y, -Ey.x);
    float2 Ni = float2(Ei.y, -Ei.x);
    float2 Ex_star = Ny * 1.f/dot(Ex,Ny);
    float2 Ey_star = Nx * 1.f/dot(Ey,Nx);
    float2 Exi_star = Ni * 1.f/dot(Ex,Ni);
    float3 dotEstarPos = float3(dot(Ex_star, pos), dot(Ey_star, pos), dot(Exi_star, pos));
    int3 dotEstarPosi = int3( dotEstarPos.x > 0 ? (uint)dotEstarPos.x : (uint)((int)dotEstarPos.x - 1),
                              dotEstarPos.y > 0 ? (uint)dotEstarPos.y : (uint)((int)dotEstarPos.y - 1),
                              dotEstarPos.z > 0 ? (uint)dotEstarPos.z : (uint)((int)dotEstarPos.z - 1) );
    uint type = (dotEstarPosi.z ^ dotEstarPosi.x ^ dotEstarPosi.y) & 1; // 0: left triangle, 1: right triangle
    float signType = (1.f - 2.f* type);
    simplex.id = uint2(dotEstarPosi.x, dotEstarPosi.y * 2 + type);
    simplex.vertices[0] = (dotEstarPosi.x + (int)type) * Ex + (dotEstarPosi.y + (int)type) * Ey ;
    simplex.vertices[1] = simplex.vertices[0] + signType * Ex;
    simplex.vertices[2] = simplex.vertices[1] + signType * Ey;
    simplex.verticesId[0] = uint2(dotEstarPosi.x + type, (dotEstarPosi.y + type) * 2);
    simplex.verticesId[1] = uint2(dotEstarPosi.x + 1 - type, (dotEstarPosi.y + type) * 2);
    simplex.verticesId[2] = uint2(dotEstarPosi.x + 1 - type, (dotEstarPosi.y + 1 - type) * 2);
    float dx = frac(dotEstarPos.x);
    float dy = frac(dotEstarPos.y);
    float dxi = frac(dotEstarPos.z);

    // for reference: simplex.barycentricCoordinates[0] = (1.f - type) - signType * frac(dotEstarPos.x);
    simplex.barycentricCoordinates[1] = type + signType * frac(dotEstarPos.z);
    simplex.barycentricCoordinates[2] = type + signType * frac(dotEstarPos.y);
    simplex.barycentricCoordinates[0] = 1.f - simplex.barycentricCoordinates[1] - simplex.barycentricCoordinates[2];

    return simplex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if 0
ptex_Simplex2D ptex_GetSimplex(in float2 pos)
{
    ptex_Simplex2D simplex;
    float2 Ex = float2(1,0);
    float2 Ey = float2(cos(PI/3.f), sin(PI/3.f));
    float2 Ei = float2(-cos(PI/3.f), sin(PI/3.f)); // 3rd edge, viewed as the "internal" diagonal of the skewed grid.
    float2 Nx = float2(0,1);
    float2 Ny = float2(Ey.y, -Ey.x);
    float2 Ni = float2(Ei.y, -Ei.x);
    float2 Ex_star = Ny * 1.f/dot(Ex,Ny);
    float2 Ey_star = Nx * 1.f/dot(Ey,Nx);
    float2 Exi_star = Ni * 1.f/dot(Ex,Ni);
    float3 dotEstarPos = float3(dot(Ex_star, pos), dot(Ey_star, pos), dot(Exi_star, pos));
    int3 dotEstarPosi = int3( dotEstarPos.x >= 0 ? (uint)dotEstarPos.x : (uint)((int)dotEstarPos.x - 1),
                              dotEstarPos.y >= 0 ? (uint)dotEstarPos.y : (uint)((int)dotEstarPos.y - 1),
                              dotEstarPos.z >= 0 ? (uint)dotEstarPos.z : (uint)((int)dotEstarPos.z - 1) );
    uint type = (dotEstarPosi.z ^ dotEstarPosi.x ^ dotEstarPosi.y) & 1; // 0: pointing up, 1: pointing down
    float signType = (1.f - 2.f* type);
    simplex.id = uint2(dotEstarPosi.x, dotEstarPosi.y * 2 + type);
    simplex.vertices[0] = (dotEstarPosi.x + (int)type) * Ex + (dotEstarPosi.y + (int)type) * Ey ;
    simplex.vertices[1] = simplex.vertices[0] + signType * Ex;
    simplex.vertices[2] = simplex.vertices[0] + signType * Ey;
    simplex.verticesId[0] = uint2(dotEstarPosi.x + type, (dotEstarPosi.y + type) * 2);
    simplex.verticesId[1] = uint2(dotEstarPosi.x + 1 - type, (dotEstarPosi.y + type) * 2);
    simplex.verticesId[2] = uint2(dotEstarPosi.x + type, (dotEstarPosi.y + 1 - type) * 2);
    float dx = frac(dotEstarPos.x);
    float dy = frac(dotEstarPos.y);
    float dxi = frac(dotEstarPos.z);

    // for reference: simplex.barycentricCoordinates[0] = (1.f - type) - signType * frac(dotEstarPos.z);
    simplex.barycentricCoordinates[1] = type + signType * frac(dotEstarPos.x);
    simplex.barycentricCoordinates[2] = type + signType * frac(dotEstarPos.y);
    simplex.barycentricCoordinates[0] = 1.f - simplex.barycentricCoordinates[1] - simplex.barycentricCoordinates[2];

    return simplex;
}
#endif
//=============================================================================
int3 ptex_GetSimplexId(in float3 pos)
{
    float3 E1 = float3(1,0,0);
    // TODO
    return int3(0,0,0);
}
//=============================================================================
int4 ptex_GetSimplexId(in float4 pos)
{
    // TODO
    return int4(0,0,0,0);
}
//=============================================================================
float ptex_ValueNoise(in float2 pos)
{
    ptex_Simplex2D simplex = ptex_GetSimplex(pos);
    uint seed = 42;
    RandomGenerator rand;
    float v[3];
    rand = rand_CreateGeneratorForId(simplex.verticesId[0], seed);
    v[0] = rand_GetNextValue(rand, 0.f, 1.f);
    rand = rand_CreateGeneratorForId(simplex.verticesId[1], seed);
    v[1] = rand_GetNextValue(rand, 0.f, 1.f);
    rand = rand_CreateGeneratorForId(simplex.verticesId[2], seed);
    v[2] = rand_GetNextValue(rand, 0.f, 1.f);
    return v[0] * simplex.barycentricCoordinates[0] + v[1] * simplex.barycentricCoordinates[1] + v[2] * simplex.barycentricCoordinates[2];
}
//=============================================================================
float4 ptex_VisualizeSimplicialGrid(in float2 pos)
{
    const bool showVertices = true;
    const bool showVertexIndex = true;
    const bool showVertexId = true;
    const bool showEdges = true;
    //-------------------------
    float4 col = float4(0,0,0,1);
    const float4 linecolor = float4(1,1,1,1);
    ptex_Simplex2D simplex = ptex_GetSimplex(pos);
    const float evalPixelThickness = 0.7f * abs(ddx(pos.x)) + abs(ddx(pos.y)) + abs(ddy(pos.x)) + abs(ddy(pos.y));

    if(showEdges)
    {
        const float thickness = 0.7f * evalPixelThickness + 0.03f;
        const float halfThickness = 0.5f * thickness;
        if(simplex.barycentricCoordinates[0] < halfThickness) col = linecolor;
        if(simplex.barycentricCoordinates[1] < halfThickness) col = linecolor;
        if(simplex.barycentricCoordinates[2] < halfThickness) col = linecolor;
        if(1-simplex.barycentricCoordinates[0] < halfThickness) col = linecolor;
        if(1-simplex.barycentricCoordinates[1] < halfThickness) col = linecolor;
        if(1-simplex.barycentricCoordinates[2] < halfThickness) col = linecolor;
    }
    if(showVertices)
    {
        float vertexRadius = 1.f * evalPixelThickness + 0.1f;
        float vertexSqRadius = vertexRadius * vertexRadius;
        RandomGenerator rand;
        float4 vertexIdColor[3];
        rand = rand_CreateGeneratorForId(simplex.verticesId[0], 0);
        vertexIdColor[0] = float4(rand_GetNextValue(rand, 0u, 4u), rand_GetNextValue(rand, 0u, 4u), rand_GetNextValue(rand, 0u, 4u), 8) / 4.f;
        rand = rand_CreateGeneratorForId(simplex.verticesId[1], 0);
        vertexIdColor[1] = float4(rand_GetNextValue(rand, 0u, 4u), rand_GetNextValue(rand, 0u, 4u), rand_GetNextValue(rand, 0u, 4u), 8) / 4.f;
        rand = rand_CreateGeneratorForId(simplex.verticesId[2], 0);
        vertexIdColor[2] = float4(rand_GetNextValue(rand, 0u, 4u), rand_GetNextValue(rand, 0u, 4u), rand_GetNextValue(rand, 0u, 4u), 8) / 4.f;
        float2 VP = simplex.vertices[0] - pos;
        if(dot(VP, VP) < vertexSqRadius) { col = showVertexId ? vertexIdColor[0] : showVertexIndex ? float4(1,0,0,1) : linecolor; }
        VP = simplex.vertices[1] - pos;
        if(dot(VP, VP) < vertexSqRadius) { col = showVertexId ? vertexIdColor[1] : showVertexIndex ? float4(0,1,0,1) : linecolor; }
        VP = simplex.vertices[2] - pos;
        if(dot(VP, VP) < vertexSqRadius) { col = showVertexId ? vertexIdColor[2] : showVertexIndex ? float4(0,0,1,1) : linecolor; }
    }
    return col;
}
//=============================================================================
float4 ptex_SimplexTest(in float2 pixelPos)
{
    float4 col = float4(0,0,0,1);
    float2 pos = pixelPos * 0.02f;
    //return pixelPos.x / 200;
    ptex_Simplex2D simplex = ptex_GetSimplex(pos);
    uint2 simplexId = simplex.id;

    uint seed = 89;

    //RandomGenerator rand = rand_CreateGeneratorForId(simplexId, seed);
    RandomGenerator rand = rand_CreateGeneratorForId((int2)pixelPos, seed);
    float c = rand_GetNextValue(rand, 0.f, 1.f);
    //return c;
    //return ((v >> 24) ^ v & 0xFF) / 255.f;
    //return (((v >> 8) & 0xFF) ^ v & 0xFF) / 255.f;
    //return (v >> 24) / 255.f;
    //return (v & 0xFF) / 255.f;

    //return frac(pos.x);
    col.rgb = (simplexId.y & 0x7) / 8.f;
    col.rgb *= 0.1f;
    //col.rgb = ((simplexId.x + simplexId.y) & 0x7) / 8.f;
    //return frac( ((simplexId.x + simplexId.y + simplexId.z) & 0xF) * 1.f / 16.f );
    //return frac( ((simplexId.x + simplexId.y) & 0xF) * 1.f / 16.f );
    //return frac( (simplexId.x & 0x7) * 1.f / 8.f);
    //return frac( (simplexId.y & 0x7) * 1.f / 8.f);
    //float2 VP = simplex.vertices[0] - pos;
    col.r = simplex.barycentricCoordinates[0];
    col.g = simplex.barycentricCoordinates[1];
    col.b = simplex.barycentricCoordinates[2];

    float t = col.r + col.g + col.b;
    col.rgb = abs(1.f - t) * 100000;

    //col = ptex_ValueNoise(pos);

    float radius = 0.15f;
    float2 VP = simplex.vertices[0] - pos;
    if(dot(VP, VP) < radius*radius) { col.rgb = float3(1,0,0); }
    VP = simplex.vertices[1] - pos;
    if(dot(VP, VP) < radius*radius) { col.rgb = float3(0,1,0); }
    VP = simplex.vertices[2] - pos;
    if(dot(VP, VP) < radius*radius) { col.rgb = float3(0,0,1); }

    //col = ptex_ValueNoise(pos);
    col = ptex_ValueNoise(pos);
    //return ptex_VisualizeSimplicialGrid(pos);
    //return ptex_PerlinSimplexGradientNoise(float3(pos.x, pos.y, 0));
    col = 0.5f + 0.5f * perlin_SimplexGradientNoise(float2(pos.x, pos.y));
    //col = 0.5f + 0.5f * perlin_SimplexGradientNoise(float3(pos.x, pos.y, 0.f));
    col = 0.5f + 0.5f * perlin_SimplexGradientNoise(float4(pos.x, pos.y, 0.25f, 0.5f));
    //float z = (date_in_frames % 50000) / 500.f; col = 0.5f + 0.5f * perlin_SimplexGradientNoise(float3(pos.x, pos.y, -0.1*z));
    //float z = (date_in_frames % 5000) / 500.f; col = abs( perlin_SimplexGradientNoise(float3(pos.x, pos.y, -z)) );
    float z = (date_in_frames % 50000) / 500.f; col = 0.5f + 0.5f * perlin_SimplexGradientNoise(float4(pos.x, pos.y, z, 0.3212*z));
    //float z = (date_in_frames % 50000) / 500.f; col = abs( perlin_SimplexGradientNoise(float4(pos.x, pos.y, z, 0.3212*z)) );
    //col = smoothstep(0.95, 1, col.x);
    //col = smoothstep(0.45, 0.55, col.x);
    //col = smoothstep(0.05, 0.15, col.x);
    if(false)
    {
        float i = 0;
        i += perlin_SimplexGradientNoise(float2(pos.x, pos.y));
        i += 0.5f * perlin_SimplexGradientNoise(2.16584645f * float2(pos.x, pos.y));
        i += 0.25f * perlin_SimplexGradientNoise(4.14565468f * float2(pos.x, pos.y));
        i += 0.125f * perlin_SimplexGradientNoise(7.95656151f * float2(pos.x, pos.y));
        i += 0.0625f * perlin_SimplexGradientNoise(16.15615481f * float2(pos.x, pos.y));
        col = 0.5f + 0.25f * i;
    }
    if(false)
    {
        float i = 0;
        i += abs(perlin_SimplexGradientNoise(float2(pos.x, pos.y)));
        i += abs(0.5f * perlin_SimplexGradientNoise(2.16584645f * float2(pos.x, pos.y)));
        i += abs(0.25f * perlin_SimplexGradientNoise(4.14565468f * float2(pos.x, pos.y)));
        i += abs(0.125f * perlin_SimplexGradientNoise(7.95656151f * float2(pos.x, pos.y)));
        i += abs(0.0625f * perlin_SimplexGradientNoise(16.15615481f * float2(pos.x, pos.y)));
        col = 0.5f * i;
    }
    //if(col.x > 254.f / 255.f) col = 1;
    //else col = 0;
    //col = pow(abs(col), 2.2f);
    return col;
}
//=============================================================================

#endif
