#ifndef Random_HLSL
#define Random_HLSL

//=============================================================================
struct RandomGenerator
{
    uint method;
    static const uint MAX_N_SEEDS = 2;
    uint seed[MAX_N_SEEDS];
    uint max_value;
    bool support_modulo;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RandomGenerator rand_CreateGenerator(in uint seed);
RandomGenerator rand_CreateGeneratorForId(in uint id, in uint seed);
RandomGenerator rand_CreateGeneratorForId(in uint2 id, in uint seed);
RandomGenerator rand_CreateGeneratorForId(in uint3 id, in uint seed);
RandomGenerator rand_CreateGeneratorForId(in uint4 id, in uint seed);
RandomGenerator rand_CreateGeneratorForPixel(in uint2 pixel, uint seed);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
uint   rand_GetNextValue(in out RandomGenerator g);
int    rand_GetNextValue(in out RandomGenerator g, in int min, in int max); // DEPRECATED
float  rand_GetNextValue(in out RandomGenerator g, in float min, in float max); // DEPRECATED
int    rand_GetUniform(in out RandomGenerator g, in int min, in int max);
float  rand_GetUniform(in out RandomGenerator g, in float min, in float max);
float2 rand_GetUniform(in out RandomGenerator g, in float2 min, in float2 max);
float3 rand_GetUniform(in out RandomGenerator g, in float3 min, in float3 max);
float4 rand_GetUniform(in out RandomGenerator g, in float4 min, in float4 max);
//=============================================================================
//                          IMPLEMENTATION
//=============================================================================
static const uint rand_Method_LinearCongruenceGenerator = 1;
static const uint rand_Method_MinimalStandardGenerator = 2;
static const uint rand_Method_DoubleLinearCongruenceGenerator = 3;
static const uint rand_Method_Hashing = 4;
#ifndef RAND_PREFERRED_METHOD
static const uint rand_PreferedMethod = rand_Method_LinearCongruenceGenerator;
#else
static const uint rand_PreferedMethod = RAND_PREFERRED_METHOD;
#endif
//=============================================================================
uint rand_Hash(in uint a)
{
    // by Thomas Wang - http://burtleburtle.net/bob/hash/integer.html
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    return a;
}
//=============================================================================
uint rand_LinearCongruentialGenerator(in uint seed)
{
    //return 1664525 * seed + 1013904223; // Numerical Recipes
    return 214013 * seed + 2531011; // MS Visual C++
}
//=============================================================================
// Minimial Standard Generator (Park & Miller) - It is a linear congruential generator
// cf. http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_rand.aspx
//     http://www.cems.uwe.ac.uk/~irjohnso/coursenotes/ufeen8-15-m/p1192-parkmiller.pdf
// implements PRN generator iteration:
//      seed = (a * seed) % m
// returns an integer between 0 and 2^31 - 1.
uint rand_MinimalStandardGenerator(in uint seed)
{
    const int m = 2147483647; // = the prime Mersenne number 2^31 - 1
    const int a = 48271; // this value would be recommanded over the original one: 16807
    const int q = m / a;
    const int r = m % a;
    const int hi = seed / q;
    const int lo = seed % q;
    seed = a * lo - r * hi;
    if ( (int)seed <= 0 )
        seed = seed + m;
    return seed;
}
//=============================================================================
// Dual phase linear congruential generator
// cf. http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_rand.aspx
uint rand_DoubleLinearCongruenceGenerator(inout uint seed[2])
{
    const int m[] = { 2147483647, 2147483399 };
    const int a[] = { 40015, 40692 };
    for(uint i = 0; i < 2; ++i)
    {
        const int q = (uint)m[i] / (uint)a[i];
        const int r = (uint)m[i] % (uint)a[i];
        const int hi = seed[i] / q;
        const int lo = seed[i] % q;
        seed[i] = a[i] * lo - r * hi;
        if ( (int)seed[i] <= 0 )
            seed[i] += m[i];
    }
    int result = seed[0] - seed[1];
    if ( result < 1 )
        result += m[0] - 1;
    return result;
}
//=============================================================================
RandomGenerator rand_CreateGenerator(in uint method, in uint seed[RandomGenerator::MAX_N_SEEDS])
{
    RandomGenerator g;
    g.method = method;
    for(uint i = 0; i < RandomGenerator::MAX_N_SEEDS; ++i)
        g.seed[i] = seed[i];
    if(rand_Method_LinearCongruenceGenerator == g.method)
    {
        g.max_value = 0xFFFFFFFF;
        g.support_modulo = false;
    }
    if(rand_Method_MinimalStandardGenerator == g.method)
    {
        g.max_value = 0x7FFFFFFE;
        g.support_modulo = false;
    }
    if(rand_Method_DoubleLinearCongruenceGenerator == g.method)
    {
        g.max_value = 0x7FFFFFFE;
        g.support_modulo = true;
    }
    if(rand_Method_Hashing == g.method)
    {
        g.max_value = 0xFFFFFFFF;
        g.support_modulo = true;
    }
    return g;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RandomGenerator rand_CreateGenerator(in uint seed)
{
    uint seeds[RandomGenerator::MAX_N_SEEDS] = { seed, 1 };
    return rand_CreateGenerator(rand_PreferedMethod, seeds );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RandomGenerator rand_CreateGeneratorForId(in uint id, in uint seed)
{
    uint compositeSeed = seed ^ id;
    if(rand_PreferedMethod != rand_Method_Hashing)
        compositeSeed = rand_Hash(compositeSeed);
    uint seeds[RandomGenerator::MAX_N_SEEDS] = { compositeSeed, 1 };
    return rand_CreateGenerator(rand_PreferedMethod, seeds );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RandomGenerator rand_CreateGeneratorForId(in uint2 id, in uint seed)
{
    id.y *= 3; // to break an ugly rotationnal symmetry around 0
    uint compositeSeed = seed ^ id.x ^ (id.y << 16) ^ (id.y >> 16);
    if(rand_PreferedMethod != rand_Method_Hashing)
        compositeSeed = rand_Hash(compositeSeed);
    uint seeds[RandomGenerator::MAX_N_SEEDS] = { compositeSeed, 1 };
    return rand_CreateGenerator(rand_PreferedMethod, seeds );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RandomGenerator rand_CreateGeneratorForId(in uint3 id, in uint seed)
{
    id.y *= 3; // to break an ugly rotationnal symmetry around 0
    id.z *= 7;
    uint compositeSeed = seed ^ id.x ^ (id.y << 10) ^ (id.y >> (32-10)) ^ (id.z << 20) ^ (id.z >> (32-20));
    if(rand_PreferedMethod != rand_Method_Hashing)
        compositeSeed = rand_Hash(compositeSeed);
    uint seeds[RandomGenerator::MAX_N_SEEDS] = { compositeSeed, 1 };
    return rand_CreateGenerator(rand_PreferedMethod, seeds );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RandomGenerator rand_CreateGeneratorForId(in uint4 id, in uint seed)
{
    id.y *= 3; // to break an ugly rotationnal symmetry around 0
    id.z *= 7;
    id.w *= 11;
    uint compositeSeed = seed ^ id.x ^ (id.y << 8) ^ (id.y >> (32-8)) ^ (id.z << 16) ^ (id.z >> (32-16)) ^ (id.w << 24) ^ (id.w >> (32-24));
    if(rand_PreferedMethod != rand_Method_Hashing)
        compositeSeed = rand_Hash(compositeSeed);
    uint seeds[RandomGenerator::MAX_N_SEEDS] = { compositeSeed, 1 };
    return rand_CreateGenerator(rand_PreferedMethod, seeds );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RandomGenerator rand_CreateGeneratorForPixel(in uint2 pixel, uint seed)
{
    uint compositeSeed = seed ^ pixel.x ^ pixel.y << 16;
    if(rand_PreferedMethod != rand_Method_Hashing)
        compositeSeed = rand_Hash(compositeSeed);
    uint seeds[RandomGenerator::MAX_N_SEEDS] = { compositeSeed, 1 };
    return rand_CreateGenerator(rand_PreferedMethod, seeds );
}
//=============================================================================
uint rand_GetNextValue(in out RandomGenerator g)
{
    if(rand_Method_LinearCongruenceGenerator == g.method)
    {
        g.seed[0] = rand_LinearCongruentialGenerator(g.seed[0]);
        return g.seed[0];
    }
    if(rand_Method_MinimalStandardGenerator == g.method)
    {
        g.seed[0] = rand_MinimalStandardGenerator(g.seed[0]);
        return g.seed[0];
    }
    if(rand_Method_DoubleLinearCongruenceGenerator == g.method)
    {
        uint seeds[2] = { g.seed[0], g.seed[1] };
        const uint result = rand_DoubleLinearCongruenceGenerator(seeds);
        g.seed[0] = seeds[0];
        g.seed[1] = seeds[1];
        return result;
    }
    if(rand_Method_Hashing == g.method)
    {
        g.seed[0] = rand_Hash(g.seed[0]);
        return g.seed[0];
    }
    return 0;
}
//=============================================================================
int rand_GetNextValue(in out RandomGenerator g, in int min, in int max)
{
    return rand_GetUniform(g, min, max);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float rand_GetNextValue(in out RandomGenerator g, in float min, in float max)
{
    return rand_GetUniform(g, min, max);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int rand_GetUniform(in out RandomGenerator g, in int min, in int max)
{
    const uint delta = max-min;
    uint v = rand_GetNextValue(g);
    if(g.support_modulo)
        v = min + v % delta;
    else
    {
        if(delta < 0x7FFF)
            v = min + (v >> 15 ^ v) % delta;
        else
            v = min + v % delta;
    }
    return v;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float rand_GetUniform(in out RandomGenerator g, in float min, in float max)
{
    uint v = rand_GetNextValue(g);
    float vf = lerp(min, max, v * (1.f / g.max_value));
    return vf;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float2 rand_GetUniform(in out RandomGenerator g, in float2 min, in float2 max)
{
    return float2(rand_GetUniform(g, min.x, max.x), rand_GetUniform(g, min.y, max.y));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float3 rand_GetUniform(in out RandomGenerator g, in float3 min, in float3 max)
{
    return float3(rand_GetUniform(g, min.x, max.x), rand_GetUniform(g, min.y, max.y), rand_GetUniform(g, min.z, max.z));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float4 rand_GetUniform(in out RandomGenerator g, in float4 min, in float4 max)
{
    return float4(rand_GetUniform(g, min.x, max.x), rand_GetUniform(g, min.y, max.y), rand_GetUniform(g, min.z, max.z), rand_GetUniform(g, min.w, max.w));
}
//=============================================================================

#endif
