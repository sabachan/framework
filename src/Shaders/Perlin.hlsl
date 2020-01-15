#ifndef Perlin_HLSL
#define Perlin_HLSL

//=============================================================================
float perlin_SimplexGradientNoise(in float2 pos);
float perlin_SimplexGradientNoise(in float3 pos);
float perlin_SimplexGradientNoise(in float4 pos);
//=============================================================================
// http://www.csee.umbc.edu/~olano/s2002c36/ch02.pdf
// These tables (permutation and grad) are not present in Perlin's article, but
// as it is kind of hard, I shortcut it using methods in
// http://webstaff.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf.
static const uint perlin_Perm[256] = {
151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

static const float3 perlin_Grad3[12] = {
    float3(1,1,0),  float3(-1,1,0), float3(1,-1,0), float3(-1,-1,0),
    float3(1,0,1),  float3(-1,0,1), float3(1,0,-1), float3(-1,0,-1),
    float3(0,1,1),  float3(0,-1,1), float3(0,1,-1), float3(0,-1,-1)};
    
static const float4 perlin_Grad4[32] = {
    float4(0,1,1,1),    float4(0,1,1,-1),   float4(0,1,-1,1),   float4(0,1,-1,-1),
    float4(0,-1,1,1),   float4(0,-1,1,-1),  float4(0,-1,-1,1),  float4(0,-1,-1,-1),
    float4(1,0,1,1),    float4(1,0,1,-1),   float4(1,0,-1,1),   float4(1,0,-1,-1),
    float4(-1,0,1,1),   float4(-1,0,1,-1),  float4(-1,0,-1,1),  float4(-1,0,-1,-1),
    float4(1,1,0,1),    float4(1,1,0,-1),   float4(1,-1,0,1),   float4(1,-1,0,-1),
    float4(-1,1,0,1),   float4(-1,1,0,-1),  float4(-1,-1,0,1),  float4(-1,-1,0,-1),
    float4(1,1,1,0),    float4(1,1,-1,0),   float4(1,-1,1,0),   float4(1,-1,-1,0),
    float4(-1,1,1,0),   float4(-1,1,-1,0),  float4(-1,-1,1,0),  float4(-1,-1,-1,0)};
//=============================================================================
float2 perlin_SimplexSkew(in float2 p)
{
    // The unit cube in skewed coordinates then corresponds to a skewed cube,
    // scaled down along its main diagonal, in original coordinates.
    // In 2D, the simplex is an equilateral triangle.
    // We seek to skew the square so that its diagonal has the same size as its
    // edges. As the skew does not modify the other diagonal, we can use the fact
    // that half this diagonal is the height of one simplex.
    // In an equilateral triangle with edge 1, h = sqrt(1 - (0.5)^2).
    const float dim = 2;
    const float h = sqrt(1 - 0.5f * 0.5f);
    const float diag = 2 * h;
    const float2 s = (diag-1)/dim * float2(1,1); // expected to be compile-time
    return p + dot(s,p) * float2(1,1);
}
float2 perlin_SimplexUnskew(in float2 p)
{
    // Inverse of preceding skew.
    // We seek s so that s_1 - s * s_1 = 1, where s_1 is the skew transformed of 1.
    const float dim = 2;
    const float s_1 = perlin_SimplexSkew(float2(1,1)).x;
    const float2 s = (s_1 - 1.f)/(s_1 * dim) * float2(1,1);
    return p - dot(s,p) * float2(1,1);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// Skew values comes from Perlin, but I have total confidence in him:
// - "For a given n, the "skew factor" f should be set to f=(n+1)^(1/2)"
float3 perlin_SimplexSkew(in float3 p)
{
    const float dim = 3;
    const float3 f = sqrt(dim + 1);
    const float3 s = (f-1)/dim * float3(1,1,1);
    return p + dot(s,p) * float3(1,1,1);
}
float3 perlin_SimplexUnskew(in float3 p)
{
    const float dim = 3;
    const float s_1 = perlin_SimplexSkew(float3(1,1,1)).x;
    const float3 s = (s_1 - 1.f)/(s_1 * dim) * float3(1,1,1);
    return p - dot(s,p) * float3(1,1,1);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float4 perlin_SimplexSkew(in float4 p)
{
    const float dim = 4;
    const float4 f = sqrt(dim + 1);
    const float4 s = (f-1)/dim * float4(1,1,1,1);
    return p + dot(s,p) * float4(1,1,1,1);
}
float4 perlin_SimplexUnskew(in float4 p)
{
    const float dim = 4;
    const float s_1 = perlin_SimplexSkew(float4(1,1,1,1)).x;
    const float4 s = (s_1 - 1.f)/(s_1 * dim) * float4(1,1,1,1);
    return p - dot(s,p) * float4(1,1,1,1);
}
//=============================================================================
float perlin_SimplexGradientNoise(in float2 pos)
{
#if 0 // DEBUG - Check skew / unskew
    {
        float2 s_pos = perlin_SimplexSkew(pos);
        float2 ss_pos = perlin_SimplexUnskew(s_pos);
        uint2 ss_ij = uint2((int)floor(ss_pos.x), (int)floor(ss_pos.y));
        RandomGenerator rand = rand_CreateGeneratorForId(ss_ij.xy, 0);
        return rand_GetNextValue(rand, 0.f, 1.f);
    }
#endif
    // Skew coordinates to generate a simplicial grid mostly regular.
    // In this skewed coordinates, 6 simplices form a cube.
    float2 s_pos = perlin_SimplexSkew(pos);
    int2 ij = int2((int)floor(s_pos.x), (int)floor(s_pos.y));
#if 0 // DEBUG - Visualize ij
    {
        //return (ij.x & 0x3) / 4.f;
        //return (ij.y & 0x3) / 4.f;
        RandomGenerator rand = rand_CreateGeneratorForId(-ij.xy-1, 0);
        return rand_GetNextValue(rand, 0.f, 1.f);
    }
#endif
    // Compute unskewed coordinates of cube corner, first vertex, V0, of simplex.
    float2 V0 = perlin_SimplexUnskew(ij);
    // pos relative to unskewed coordinate of first vertex.
    float2 uv0 = pos - V0;
    // Determine which simplex we're in. A is the offset of the second corner
    // of vertex in skewed coords.
    int2 s_V0V1;
    if(uv0.x > uv0.y) s_V0V1 = int2(1,0);
    else s_V0V1 = int2(0,1);
    const int2 s_V0V2 = int2(1,1);
    // Determines the two other simplex vertices V1 and V2.
    float2 V1 = V0 + perlin_SimplexUnskew(s_V0V1);
    float2 V2 = V0 + perlin_SimplexUnskew(s_V0V2);
#if 0 // DEBUG - check vertices
    {
        float col = 0;
        const float vertexRadius = 0.15f;
        const float vertexSqRadius = vertexRadius * vertexRadius;
        float2 VP = pos - V0;
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = pos - V1;
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = pos - V2;
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        return col;
    }
#endif
    // Compute pos relative to other vertices.
    // for reference: float2 uv1 = pos - V1;
    float2 uv1 = uv0 - perlin_SimplexUnskew(s_V0V1);
    // for reference: float2 uv2 = pos - V2;
    float2 uv2 = uv0 - perlin_SimplexUnskew(int2(1,1));
#if 0 // DEBUG - check uv
    {
        return uv0.y;
        return abs(uv1.y);
    }
#endif
    // -- end of simplicial grid (to extract)

    //==== From here, I use code from "simplex noise demystified", without really undertanding everything

    // Compute pseudo-random gradients indices (differs from Perlin article)
    uint ii = ij.x & 0xFF;
    uint jj = ij.y & 0xFF;
    uint i1 = s_V0V1.x;
    uint j1 = s_V0V1.y;
    uint gi0 = perlin_Perm[(ii+perlin_Perm[jj & 0xFF]) & 0xFF] % 12;
    uint gi1 = perlin_Perm[(ii+i1+perlin_Perm[(jj+j1) & 0xFF]) & 0xFF] % 12;
    uint gi2 = perlin_Perm[(ii+1+perlin_Perm[(jj+1) & 0xFF]) & 0xFF] % 12;
#if 0 // DEBUG - check vertices gradient indices
    {
        float col = 0;
        const float vertexRadius = 0.15f;
        const float vertexSqRadius = vertexRadius * vertexRadius;
        float2 VP = pos - V0;
        if(dot(VP, VP) < vertexSqRadius) { col = gi0 / 12.f; }
        VP = pos - V1;
        if(dot(VP, VP) < vertexSqRadius) { col = gi1 / 12.f; }
        VP = pos - V2;
        if(dot(VP, VP) < vertexSqRadius) { col = gi2 / 12.f; }
        return col;
    }
#endif
#if 0 // DEBUG - check 1 vertex gradients
    {
        float2 s_VToShow = float2(0,0);
        float2 VToShow = perlin_SimplexUnskew(s_VToShow);
        float2 VP = VToShow - V0;
        float2 g = float2(0,0);
        float2 uv = float2(1,1);
        if(dot(VToShow-V0, VToShow-V0) < 0.01f) { g = perlin_Grad3[gi0].xy; uv = uv0; }
        if(dot(VToShow-V1, VToShow-V1) < 0.01f) { g = perlin_Grad3[gi1].xy; uv = uv1; }
        if(dot(VToShow-V2, VToShow-V2) < 0.01f) { g = perlin_Grad3[gi2].xy; uv = uv2; }
        //return dot(g, uv); // show gradient
        float n;
        float t = 0.5f - dot(uv, uv);
        //if(t > 0) return 1; // show contribution
        if(t < 0) {
            n = 0.f;
            t = 0.f;
        } else {
            t *= t;
            t *= t;
            n = t * dot(g, uv);
        }
        // let x be the distance to vertex.
        // then, P(x) = (0.5-x*x)^4 = (0.5^2-x^2+x^4)^2 = 0.5^4 - x^2 + 2x^4 - 2x^6 + x^8
        return 70.f * t * dot(g, uv);
        return 16.f * t;
    }
#endif
    // Calculate the contribution from the three corners.
    float n0;
    float t0 = 0.5f - dot(uv0, uv0);
    if(t0 < 0) n0 = 0.f;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(perlin_Grad3[gi0].xy, uv0);  // (x,y) of grad3 used for 2D gradient
    }
    float n1;
    float t1 = 0.5f - dot(uv1, uv1);
    if(t1<0) n1 = 0.f;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(perlin_Grad3[gi1].xy, uv1);
    }
    float n2;
    float t2 = 0.5f - dot(uv2, uv2);
    if(t2<0) n2 = 0.f;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(perlin_Grad3[gi2].xy, uv2);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.f * (n0 + n1 + n2);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float perlin_SimplexGradientNoise(in float3 pos)
{
#if 0 // DEBUG - Check skew / unskew
    {
        float3 s_pos = perlin_SimplexSkew(pos);
        float3 ss_pos = perlin_SimplexUnskew(s_pos);
        uint3 ss_ijk = uint3((int)floor(ss_pos.x), (int)floor(ss_pos.y), (int)floor(ss_pos.z));
        RandomGenerator rand = rand_CreateGeneratorForId((int3)ss_ijk.xyz, 0);
        return rand_GetNextValue(rand, 0.f, 1.f);
    }
#endif
    // Skew coordinates to generate a simplicial grid mostly regular.
    // In this skewed coordinates, 6 simplices form a cube.
    float3 s_pos = perlin_SimplexSkew(pos);
    int3 ijk = int3((int)floor(s_pos.x), (int)floor(s_pos.y), (int)floor(s_pos.z));
#if 0 // DEBUG - Visualize ijk
    {
        //return (ijk.x & 0x3) / 4.f;
        //return (ijk.y & 0x3) / 4.f;
        RandomGenerator rand = rand_CreateGeneratorForId(ijk.xyz, 0);
        return rand_GetNextValue(rand, 0.f, 1.f);
    }
#endif
    // Compute unskewed coordinates of cube corner, first vertex, V0, of simplex.
    float3 V0 = perlin_SimplexUnskew(ijk);
    // pos relative to unskewed coordinate of first vertex.
    float3 uv0 = pos - V0;
    // Determine which simplex we're in. A is the offset of the second corner
    // of vertex in skewed coords.
    int3 s_V0V1;
    int3 s_V0V2;
    if(uv0.x > uv0.y) {
        if(uv0.x > uv0.z) {
            if(uv0.y > uv0.z) { s_V0V1 = int3(1,0,0); s_V0V2 = int3(1,1,0); }
            else              { s_V0V1 = int3(1,0,0); s_V0V2 = int3(1,0,1); }
        } else                { s_V0V1 = int3(0,0,1); s_V0V2 = int3(1,0,1); }
    } else {
        if(uv0.x > uv0.z)     { s_V0V1 = int3(0,1,0); s_V0V2 = int3(1,1,0); }
        else {
            if(uv0.y > uv0.z) { s_V0V1 = int3(0,1,0); s_V0V2 = int3(0,1,1); }
            else              { s_V0V1 = int3(0,0,1); s_V0V2 = int3(0,1,1); }
        }
    }
    const int3 s_V0V3 = int3(1,1,1);
    // Determines the two other simplex vertices V1 and V2.
    float3 V1 = V0 + perlin_SimplexUnskew(s_V0V1);
    float3 V2 = V0 + perlin_SimplexUnskew(s_V0V2);
    float3 V3 = V0 + perlin_SimplexUnskew(s_V0V3);
#if 0 // DEBUG - check vertices
    {
        float col = 0;
        const float vertexRadius = 0.15f;
        const float vertexSqRadius = vertexRadius * vertexRadius;
        float3 VP = (pos - V0) * float3(1,1,0.5f);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = (pos - V1) * float3(1,1,0.5f);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = (pos - V2) * float3(1,1,0.5f);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = (pos - V3) * float3(1,1,0.5f);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        return col;
    }
#endif
    // Compute pos relative to other vertices.
    float3 uv1 = uv0 - perlin_SimplexUnskew(s_V0V1);
    float3 uv2 = uv0 - perlin_SimplexUnskew(s_V0V2);
    float3 uv3 = uv0 - perlin_SimplexUnskew(s_V0V3);
#if 0 // DEBUG - check uv
    {
        //return uv1.x;
        return abs(uv1.y);
    }
#endif
    // -- end of simplicial grid (to extract)

    //==== From here, I use code from "simplex noise demystified", without really undertanding everything

    // Compute pseudo-random gradients indices (differs from Perlin article)
    uint ii = ijk.x & 0xFF;
    uint jj = ijk.y & 0xFF;
    uint kk = ijk.z & 0xFF;
    uint i1 = s_V0V1.x;
    uint j1 = s_V0V1.y;
    uint k1 = s_V0V1.z;
    uint i2 = s_V0V2.x;
    uint j2 = s_V0V2.y;
    uint k2 = s_V0V2.z;
    uint gi0 = perlin_Perm[(ii   +perlin_Perm[(jj   +perlin_Perm[ kk     & 0xFF]) & 0xFF]) & 0xFF] % 12;
    uint gi1 = perlin_Perm[(ii+i1+perlin_Perm[(jj+j1+perlin_Perm[(kk+k1) & 0xFF]) & 0xFF]) & 0xFF] % 12;
    uint gi2 = perlin_Perm[(ii+i2+perlin_Perm[(jj+j2+perlin_Perm[(kk+k2) & 0xFF]) & 0xFF]) & 0xFF] % 12;
    uint gi3 = perlin_Perm[(ii+ 1+perlin_Perm[(jj+ 1+perlin_Perm[(kk+ 1) & 0xFF]) & 0xFF]) & 0xFF] % 12;
#if 0 // DEBUG - check vertices gradient indices
    {
        float col = 0;
        const float vertexRadius = 0.15f;
        const float vertexSqRadius = vertexRadius * vertexRadius;
        float3 VP = pos - V0;
        if(dot(VP, VP) < vertexSqRadius) { col = gi0 / 12.f; }
        VP = pos - V1;
        if(dot(VP, VP) < vertexSqRadius) { col = gi1 / 12.f; }
        VP = pos - V2;
        if(dot(VP, VP) < vertexSqRadius) { col = gi2 / 12.f; }
        VP = pos - V3;
        if(dot(VP, VP) < vertexSqRadius) { col = gi3 / 12.f; }
        return col;
    }
#endif
#if 0 // DEBUG - check 1 vertex gradients
    {
        gi0 = 5;
        gi1 = 5;
        gi2 = 5;
        gi3 = 5;
        float3 s_VToShow = float3(0,0,0);
        float3 VToShow = perlin_SimplexUnskew(s_VToShow);
        float3 VP = VToShow - V0;
        float3 g = float3(0,0,0);
        float3 uv = float3(1,1,1);
        if(dot(VToShow-V0, VToShow-V0) < 0.01f) { g = perlin_Grad3[gi0]; uv = uv0; }
        if(dot(VToShow-V1, VToShow-V1) < 0.01f) { g = perlin_Grad3[gi1]; uv = uv1; }
        if(dot(VToShow-V2, VToShow-V2) < 0.01f) { g = perlin_Grad3[gi2]; uv = uv2; }
        if(dot(VToShow-V3, VToShow-V3) < 0.01f) { g = perlin_Grad3[gi3]; uv = uv3; }
        //return dot(g, uv); // show gradient
        float n;
        float t = 0.5f - dot(uv, uv);
        if(t > 0) return 1; // show contribution
        if(t < 0) {
            n = 0.f;
            t = 0.f;
        } else {
            t *= t;
            t *= t;
            n = t * dot(g, uv);
        }
        return 32.f * t * dot(g, uv);
        return 16.f * t;
    }
#endif
    // Calculate the contribution from the 4 corners.
#define COMPUTE_CORNER_CONTRIBUTION(i) \
    float n##i; \
    float t##i = 0.6f - dot(uv##i, uv##i); \
    if(t##i < 0) n##i = 0.f; \
    else { \
        t##i *= t##i; \
        n##i = t##i * t##i * dot(perlin_Grad3[gi##i], uv##i); \
    }
    
    COMPUTE_CORNER_CONTRIBUTION(0)
    COMPUTE_CORNER_CONTRIBUTION(1)
    COMPUTE_CORNER_CONTRIBUTION(2)
    COMPUTE_CORNER_CONTRIBUTION(3)
#undef COMPUTE_CORNER_CONTRIBUTION
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 32.f * (n0 + n1 + n2 + n3);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float perlin_SimplexGradientNoise(in float4 pos)
{
#if 0 // DEBUG - Check skew / unskew
    {
        float4 s_pos = perlin_SimplexSkew(pos);
        float4 ss_pos = perlin_SimplexUnskew(s_pos);
        uint4 ss_ij = uint4((int)floor(ss_pos.x), (int)floor(ss_pos.y), (int)floor(ss_pos.z), (int)floor(ss_pos.w));
        RandomGenerator rand = rand_CreateGeneratorForId((int4)ss_ij.xyzw, 0);
        return rand_GetNextValue(rand, 0.f, 1.f);
    }
#endif
    // Skew coordinates to generate a simplicial grid mostly regular.
    // In this skewed coordinates, 6 simplices form a cube.
    float4 s_pos = perlin_SimplexSkew(pos);
    int4 ij = int4((int)floor(s_pos.x), (int)floor(s_pos.y), (int)floor(s_pos.z), (int)floor(s_pos.w));
#if 0 // DEBUG - Visualize ij
    {
        //return (ij.x & 0x3) / 4.f;
        //return (ij.y & 0x3) / 4.f;
        RandomGenerator rand = rand_CreateGeneratorForId(ij.xyzw, 0);
        return rand_GetNextValue(rand, 0.f, 1.f);
    }
#endif
    // Compute unskewed coordinates of cube corner, first vertex, V0, of simplex.
    float4 V0 = perlin_SimplexUnskew(ij);
    // pos relative to unskewed coordinate of first vertex.
    float4 uv0 = pos - V0;
    // Determine which simplex we're in.
    int4 rank = int4(0,1,2,3);
    if(uv0.x < uv0.y) rank.xy += int2(1,-1);
    if(uv0.x < uv0.z) rank.xz += int2(1,-1);
    if(uv0.x < uv0.w) rank.xw += int2(1,-1);
    if(uv0.y < uv0.z) rank.yz += int2(1,-1);
    if(uv0.y < uv0.w) rank.yw += int2(1,-1);
    if(uv0.z < uv0.w) rank.zw += int2(1,-1);
    int4 s_V0V1 = int4(rank.x < 1 ? 1 : 0, rank.y < 1 ? 1 : 0, rank.z < 1 ? 1 : 0, rank.w < 1 ? 1 : 0);
    int4 s_V0V2 = int4(rank.x < 2 ? 1 : 0, rank.y < 2 ? 1 : 0, rank.z < 2 ? 1 : 0, rank.w < 2 ? 1 : 0);
    int4 s_V0V3 = int4(rank.x < 3 ? 1 : 0, rank.y < 3 ? 1 : 0, rank.z < 3 ? 1 : 0, rank.w < 3 ? 1 : 0);
    const int4 s_V0V4 = int4(1,1,1,1);
    // Determines other simplex vertices.
    float4 V1 = V0 + perlin_SimplexUnskew(s_V0V1);
    float4 V2 = V0 + perlin_SimplexUnskew(s_V0V2);
    float4 V3 = V0 + perlin_SimplexUnskew(s_V0V3);
    float4 V4 = V0 + perlin_SimplexUnskew(s_V0V4);
#if 0 // DEBUG - check vertices
    {
        float col = 0;
        const float vertexRadius = 0.14f;
        const float vertexSqRadius = vertexRadius * vertexRadius;
        float4 VP = (pos - V0) * float4(1,1,1,1);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = (pos - V1) * float4(1,1,1,1);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = (pos - V2) * float4(1,1,1,1);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = (pos - V3) * float4(1,1,1,1);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        VP = (pos - V4) * float4(1,1,1,1);
        if(dot(VP, VP) < vertexSqRadius) { col = 1; }
        return col;
    }
#endif
    // Compute pos relative to other vertices.
    float4 uv1 = uv0 - perlin_SimplexUnskew(s_V0V1);
    float4 uv2 = uv0 - perlin_SimplexUnskew(s_V0V2);
    float4 uv3 = uv0 - perlin_SimplexUnskew(s_V0V3);
    float4 uv4 = uv0 - perlin_SimplexUnskew(s_V0V4);
#if 0 // DEBUG - check uv
    {
        //return uv1.x;
        return abs(uv1.y);
    }
#endif
    // -- end of simplicial grid (to extract)

    //==== From here, I use code from "simplex noise demystified", without really undertanding everything

    // Compute pseudo-random gradients indices (differs from Perlin article)
    uint ii = ij.x & 0xFF;
    uint jj = ij.y & 0xFF;
    uint kk = ij.z & 0xFF;
    uint ll = ij.w & 0xFF;
    uint i1 = s_V0V1.x;
    uint j1 = s_V0V1.y;
    uint k1 = s_V0V1.z;
    uint l1 = s_V0V1.w;
    uint i2 = s_V0V2.x;
    uint j2 = s_V0V2.y;
    uint k2 = s_V0V2.z;
    uint l2 = s_V0V2.w;
    uint i3 = s_V0V3.x;
    uint j3 = s_V0V3.y;
    uint k3 = s_V0V3.z;
    uint l3 = s_V0V3.w;
    uint gi0 = perlin_Perm[(ii   +perlin_Perm[(jj   +perlin_Perm[( kk    + perlin_Perm[ ll     & 0xFF]) & 0xFF]) & 0xFF]) & 0xFF] % 32;
    uint gi1 = perlin_Perm[(ii+i1+perlin_Perm[(jj+j1+perlin_Perm[((kk+k1)+ perlin_Perm[(ll+l1) & 0xFF]) & 0xFF]) & 0xFF]) & 0xFF] % 32;
    uint gi2 = perlin_Perm[(ii+i2+perlin_Perm[(jj+j2+perlin_Perm[((kk+k2)+ perlin_Perm[(ll+l2) & 0xFF]) & 0xFF]) & 0xFF]) & 0xFF] % 32;
    uint gi3 = perlin_Perm[(ii+i3+perlin_Perm[(jj+j3+perlin_Perm[((kk+k3)+ perlin_Perm[(ll+l3) & 0xFF]) & 0xFF]) & 0xFF]) & 0xFF] % 32;
    uint gi4 = perlin_Perm[(ii+ 1+perlin_Perm[(jj+ 1+perlin_Perm[((kk+ 1)+ perlin_Perm[(ll+ 1) & 0xFF]) & 0xFF]) & 0xFF]) & 0xFF] % 32;
#if 0 // DEBUG - check vertices gradient indices
    {
        float col = 0;
        const float vertexRadius = 0.14f;
        const float vertexSqRadius = vertexRadius * vertexRadius;
        float4 VP = pos - V0;
        if(dot(VP, VP) < vertexSqRadius) { col = gi0 / 32.f; }
        VP = pos - V1;
        if(dot(VP, VP) < vertexSqRadius) { col = gi1 / 32.f; }
        VP = pos - V2;
        if(dot(VP, VP) < vertexSqRadius) { col = gi2 / 32.f; }
        VP = pos - V3;
        if(dot(VP, VP) < vertexSqRadius) { col = gi3 / 32.f; }
        VP = pos - V4;
        if(dot(VP, VP) < vertexSqRadius) { col = gi4 / 32.f; }
        return col;
    }
#endif
#if 0 // DEBUG - check 1 vertex gradients
    {
        gi0 = 15;
        gi1 = 15;
        gi2 = 15;
        gi3 = 15;
        gi4 = 15;
        float4 s_VToShow = float4(0,0,0,0);
        float4 VToShow = perlin_SimplexUnskew(s_VToShow);
        float4 VP = VToShow - V0;
        float4 g = float4(0,0,0,0);
        float4 uv = float4(1,1,1,1);
        if(dot(VToShow-V0, VToShow-V0) < 0.01f) { g = perlin_Grad4[gi0]; uv = uv0; }
        if(dot(VToShow-V1, VToShow-V1) < 0.01f) { g = perlin_Grad4[gi1]; uv = uv1; }
        if(dot(VToShow-V2, VToShow-V2) < 0.01f) { g = perlin_Grad4[gi2]; uv = uv2; }
        if(dot(VToShow-V3, VToShow-V3) < 0.01f) { g = perlin_Grad4[gi3]; uv = uv3; }
        if(dot(VToShow-V4, VToShow-V4) < 0.01f) { g = perlin_Grad4[gi4]; uv = uv4; }
        //return dot(g, uv); // show gradient
        float n;
        float t = 0.6f - dot(uv, uv);
        //if(t > 0) return 1; // show contribution
        if(t < 0) {
            n = 0.f;
            t = 0.f;
        } else {
            t *= t;
            t *= t;
            n = t * dot(g, uv);
        }
        return 27.f * t * dot(g, uv);
    }
#endif
    // Calculate the contribution from the 5 corners.
#define COMPUTE_CORNER_CONTRIBUTION(i) \
    float n##i; \
    float t##i = 0.6f - dot(uv##i, uv##i); \
    if(t##i < 0) n##i = 0.f; \
    else { \
        t##i *= t##i; \
        n##i = t##i * t##i * dot(perlin_Grad4[gi##i], uv##i); \
    }
    
    COMPUTE_CORNER_CONTRIBUTION(0)
    COMPUTE_CORNER_CONTRIBUTION(1)
    COMPUTE_CORNER_CONTRIBUTION(2)
    COMPUTE_CORNER_CONTRIBUTION(3)
    COMPUTE_CORNER_CONTRIBUTION(4)
#undef COMPUTE_CORNER_CONTRIBUTION
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 27.f * (n0 + n1 + n2 + n3 + n4);
}
//=============================================================================
float2 perlin_SimplexGradientNoiseAndWeight(in float2 pos) { return float2(perlin_SimplexGradientNoise(pos), 1); }
float2 perlin_SimplexGradientNoiseAndWeight(in float3 pos) { return float2(perlin_SimplexGradientNoise(pos), 1); }
//=============================================================================
#endif
