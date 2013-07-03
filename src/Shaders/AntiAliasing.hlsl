#ifndef AntiAliasing_HLSL
#define AntiAliasing_HLSL

//=============================================================================
#define aa_true 1
#define aa_false 0
#define aa_undefined -1
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct aa_Method
{
    int useEllipticSamplingIFP;
    int useDistanceFieldIFP;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Method aa_CreateAutomaticMethod()
{
    aa_Method m;
    m.useEllipticSamplingIFP = aa_true;
    m.useDistanceFieldIFP = aa_true;
    return m;
};
//=============================================================================
struct aa_Fragment
{
    float2 pos;
    float2 dposdu;
    float2 dposdv;
    float aaradius;
    aa_Method aamethod;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Fragment aa_Create2DFragmentFromPos(in float2 pos)
{
    aa_Fragment f;
    f.pos = pos;
    f.dposdu = ddx(pos);
    f.dposdv = ddy(pos);
    f.aaradius = 1;
    f.aamethod = aa_CreateAutomaticMethod();
    return f;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Fragment aa_Create2DFragmentFromPosRadius(in float2 pos, in float radius)
{
    aa_Fragment f;
    f.pos = pos;
    f.dposdu = float2(radius, 0);
    f.dposdv = float2(0, radius);
    f.aaradius = 1;
    f.aamethod = aa_CreateAutomaticMethod();
    return f;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Fragment mul(in aa_Fragment frg, in float2 v)
{
    aa_Fragment f;
    f.pos = frg.pos * v;
    f.dposdu = frg.dposdu * v;
    f.dposdv = frg.dposdv * v;
    f.aaradius = frg.aaradius;
    f.aamethod = aa_CreateAutomaticMethod();
    return f;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Fragment mul(in aa_Fragment frg, in float s) { return mul(frg, float2(s,s)); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Fragment mul(in float2x2 m, in aa_Fragment frg)
{
    aa_Fragment f;
    f.pos = mul(m, frg.pos);
    f.dposdu = mul(m, frg.dposdu);
    f.dposdv = mul(m, frg.dposdv);
    f.aaradius = frg.aaradius;
    f.aamethod = frg.aamethod;
    return f;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Fragment add(in aa_Fragment frg, in float2 v)
{
    aa_Fragment f;
    f.pos = frg.pos + v;
    f.dposdu = frg.dposdu;
    f.dposdv = frg.dposdv;
    f.aaradius = frg.aaradius;
    f.aamethod = frg.aamethod;
    return f;
}
//=============================================================================
struct aa_Intensity
{
    float value;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity aa_CreateIntensity(in float intensity)
{
    aa_Intensity i;
    i.value = intensity;
    return i;
}
aa_Intensity aa_inv(in aa_Intensity a)                      { return aa_CreateIntensity( 1.f-a.value ); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity aa_unaligned_inter(in aa_Intensity a, in aa_Intensity b) { return aa_CreateIntensity( a.value * b.value ); }
aa_Intensity aa_unaligned_union(in aa_Intensity a, in aa_Intensity b) { return aa_CreateIntensity( a.value + b.value - (a.value * b.value) ); }
aa_Intensity aa_unaligned_sub(in aa_Intensity a, in aa_Intensity b)   { return aa_unaligned_inter( a, aa_inv(b) ); }
aa_Intensity aa_unaligned_xor(in aa_Intensity a, in aa_Intensity b)   { return aa_unaligned_sub( aa_unaligned_union( a, b ), aa_unaligned_inter( a, b) ); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity aa_aligned_inter(in aa_Intensity a, in aa_Intensity b) { return aa_CreateIntensity( min(a.value, b.value) ); }
aa_Intensity aa_aligned_union(in aa_Intensity a, in aa_Intensity b) { return aa_CreateIntensity( max(a.value, b.value) ); }
aa_Intensity aa_aligned_sub(in aa_Intensity a, in aa_Intensity b)   { return aa_aligned_inter( a, aa_inv(b) ); }
aa_Intensity aa_aligned_xor(in aa_Intensity a, in aa_Intensity b)   { return aa_aligned_sub( aa_aligned_union( a, b ), aa_aligned_inter( a, b) ); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity aa_inter(in aa_Intensity a, in aa_Intensity b) { return aa_aligned_inter(a, b); }
aa_Intensity aa_union(in aa_Intensity a, in aa_Intensity b) { return aa_aligned_union(a, b); }
aa_Intensity aa_sub(in aa_Intensity a, in aa_Intensity b)   { return aa_aligned_sub(a, b); }
aa_Intensity aa_xor(in aa_Intensity a, in aa_Intensity b)   { return aa_aligned_xor(a, b); }
//=============================================================================
aa_Intensity aa_Heaviside(in float value, in float delta)
{
    float hdelta = 0.5f * delta;
    return aa_CreateIntensity(smoothstep(-hdelta, hdelta, value));
}
//=============================================================================
// rect beetween [-1, 1]
aa_Intensity aa_CenteredUnitRect(in float value, in float delta)
{
    float hdelta = 0.5f * delta;
    float i = smoothstep(-hdelta, hdelta, value+1) - smoothstep(-hdelta, hdelta, value-1);
    return aa_CreateIntensity(i);
}
//=============================================================================
// f(value) is periodic, with period 1, with f = 1 when value in [0, alpha[, f = 0 when value in [alpha, 1[
aa_Intensity aa_UnitSquareWave(in float value, in float delta, in float alpha)
{
    // In [-1/2, 1/2], nearest up transition is in 0. In this range,
    // if value < -1/2+alpha, nearest down transition is in -1+alpha,
    // while if value > -1/2+alpha, nearest down transition is in alpha.
    float i = 0;
    float average = alpha;
    float hdelta = 0.5f * delta;
    float m0505value = frac(value+0.5f)-0.5f;
    if(m0505value < -0.5f+alpha)
    {
        i = 1 - smoothstep(-hdelta, hdelta, m0505value+1-alpha) + smoothstep(-hdelta, hdelta, m0505value);
    }
    else
    {
        i = smoothstep(-hdelta, hdelta, m0505value) - smoothstep(-hdelta, hdelta, m0505value-alpha);
    }
    i = lerp(average, i, smoothstep(1., 0.9, delta));
    return aa_CreateIntensity(i);
}
//=============================================================================

#endif
