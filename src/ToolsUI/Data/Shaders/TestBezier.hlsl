#include "../../../Shaders/Bezier.hlsl"
#include "../../../Shaders/Geometry.hlsl"

uniform uint date_in_frames;

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
};

struct p2f {
    float4 Col : SV_Target0;
};
//=============================================================================
float4 Blend(in float4 dst, in float4 src)
{
    float4 r = dst * (1 - src.a) + src;
    return r;
}
//=============================================================================
struct BezierCurve
{
    float2 P0;
    float2 P1;
    float2 P2;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BezierCurve CreateBezierCurve_0()
{
    BezierCurve curve;
    curve.P0 = float2(100, 100);
    curve.P1 = float2(500, 100);
    curve.P2 = float2(300, 500);
    return curve;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BezierCurve CreateBezierCurve()
{
    BezierCurve curve;
    int date = date_in_frames;
    float a = date/3. / 10;
    float b = date/3. / 13.1;
    float c = date/3. / 16.1;
    curve.P0 = float2(100, 100) + float2(cos(a), sin(a)) * 10;
    curve.P1 = float2(500, 100) + float2(cos(b), sin(b)) * 100;
    curve.P2 = float2(300, 500) + float2(cos(c), sin(c)) * 10;
    return curve;
}
//=============================================================================
float4 Render(in BezierCurve curve, in aa_Fragment frg)
{
    float4 col = float4(0,0,0,0);
    float pixel = frg.dposdu.x;

#if 1
    {
        aa_Intensity i = aa_CreateIntensity(0);
        i = aa_unaligned_union(i, geom_Segment(frg, curve.P0, curve.P1, 80*pixel));
        i = aa_unaligned_union(i, geom_Segment(frg, curve.P1, curve.P2, 80*pixel));
        int N = 10;
        for(int k = 0; k < N; ++k)
        {
            float t = float(k)/N;
            float2 A = lerp(curve.P0, curve.P1, t);
            float2 B = lerp(curve.P1, curve.P2, t);
            i = aa_unaligned_union(i, geom_Segment(frg, A, B, 1*pixel));
        }
        col = lerp(col, float4(1,1,1,1)*0.03, i.value);
    }
#endif
    
    float2 PB = bezier_ComputeDistanceVectorToQuadratic(curve.P0, curve.P1, curve.P2, frg.pos);
    float radius = 40;
#if 0
    float d2 = dot(PB, PB);
    if(d2 < radius*radius) col = float4(1,1,1,1);
#else
    aa_Intensity i = aa_CreateIntensity(0);
    i = aa_unaligned_union(i, geom_Disc(frg, frg.pos+PB, radius));
    col = lerp(col, float4(1,1,1,1), i.value);
#endif
    return col;
}
//=============================================================================
void pmain(in v2p IN, out p2f OUT)
{
    float4 col = float4(0,0,0,0);
    
    float w = 1.f/ddx(IN.Tex.x);
    float h = 1.f/ddy(IN.Tex.y);
    float ratio = w/h;
    float2 wh = float2(w,h);
    
    aa_Fragment frg = aa_Create2DFragmentFromPos(IN.Tex * wh);
    frg.aaradius = 1.5;
    frg.aamethod.useDistanceFieldIFP = 1;
    frg.aamethod.useEllipticSamplingIFP = 1;

    BezierCurve curve = CreateBezierCurve();
    col = Blend(col, Render(curve, frg));
#if 0
    float2 C = float2(300, 300);
    float R = 100;
    aa_Intensity i = geom_Disc(frg, C,  R);
    i = aa_aligned_union(i, geom_Box_MinMax(frg, C - R * float2(1, 2), C + R * float2(1, 0) + frg.aaradius * float2(0, 0.5f)));
    i = aa_unaligned_sub(i, geom_EllipticDisc_CenterHalfAxisRatio(frg, C,  float2(50, 50), 0.5f));
    i = aa_unaligned_union(i, geom_Circle(frg, C + R * float2(1, 2), R, 20));
    col = Blend(col, float4(1,1,1,1) * i.value);
#endif

    OUT.Col = col;
}
