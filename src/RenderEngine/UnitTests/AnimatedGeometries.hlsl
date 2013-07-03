#include "../../Shaders/Geometry.hlsl"
#include "../../Shaders/CirclesFont.hlsl"
#include "../../Shaders/ProceduralTexture.hlsl"

Texture2D<float4> texture_input;
SamplerState sampler_input;

//uniform uint date_in_frames;

struct a2v {
    float2 Pos : POSITION;
    float2 Tex : TEXTURE;
};

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
};

struct p2f {
    float4 Col : SV_Target0;
};

void vmain(in a2v IN, out v2p OUT)
{
    OUT.Pos.xy = IN.Pos;
    OUT.Pos.z = 1.f-1.e-5f;
    OUT.Pos.w = 1.f;
    OUT.Tex = IN.Tex;
}

void pmain(in v2p IN, out p2f OUT)
{
    OUT.Col.rg = IN.Tex.xy;
    OUT.Col.b = 0;
    OUT.Col.a = 1;

    // Note: As we know that render target resolution is an integer, we can
    // compute its exact value by rounding approximation using ddx(texCoords):
    uint w = 1.f/ddx(IN.Tex.x) + 0.5f;
    uint h = 1.f/ddy(IN.Tex.y) + 0.5f;

    float ratio = w/h;
    float2 wh = float2(w,h);

    float4 texCol = float4(0,0,0,0);
#if 1
    {
        aa_Fragment frg = aa_Create2DFragmentFromPos(IN.Pos.xy);
        frg.aaradius = 2.001;
        frg.aaradius = 1.999;
        frg.aaradius = 1.f;
        aa_Intensity i = aa_CreateIntensity(0);

        i = aa_union(i, geom_Line_PointNormal(frg, 0.5f*wh, float2(1,0), 1.f));
        i = aa_union(i, geom_Line_PointNormal(frg, 0.5f*wh, float2(0,1), 1.f));
        i = aa_sub(i, geom_Disc(frg, 0.5f*wh, 0.05f*h));

        i = aa_union(i, geom_Circle(frg, 0.5f*wh, 0.15f*h, 1.f));
        i = aa_union(i, geom_Circle(frg, 0.5f*wh, 0.15f*h+10, 2.5f));

        aa_Intensity i2 = aa_CreateIntensity(0);
        i2 = aa_union(i2, geom_Circle(frg, 0.5f*wh, 0.15f*h+5, 10.f));
        //i2.value *= 0.2f;

        float f = ((IN.Tex.x+0.5f*IN.Tex.y) * w - 50) / 3.12347;
        float dfdx = ddx(f);
        float dfdy = ddy(f);
        float delta = sqrt(dfdx*dfdx + dfdy*dfdy) * frg.aaradius;
        i2 = aa_inter( i2, aa_UnitSquareWave(f, delta, 0.3));

        float date = date_in_frames;
        //date = 101;
        float angle = date * 3.14 / 300.f;
        float c = cos(angle);
        float s = sin(angle);
        float2x2 R = {c, -s, s, c};
        float2 N1 = mul(R, float2(1,0));
        float2 N2 = mul(R, float2(-1,1.5));
        i2 = aa_inter(i2, aa_xor(geom_HalfPlane_PointNormal(frg, 0.5f*wh, N1), geom_HalfPlane_PointNormal(frg, 0.5f*wh, N2)));

        i = aa_union(i, i2);

        #if 0
        i = aa_CreateIntensity(0);
        i = aa_union(i, geom_HalfPlane_PointNormal(frg, float2(500,0), float2(-100,400)));
        i = aa_union(i, geom_Line_PointNormal(frg, float2(0,300), float2(1,4), 1.f));
        i = aa_union(i, geom_Disc(frg, float2(250, 100), 50));
        i = aa_union(i, geom_Circle(frg, float2(100, 100), 50, 1.f));
        i = aa_union(i, geom_Parallelogram_CenterEdges(frg, float2(300, 300), float2(40, 10), float2(40, 30)));
        i = aa_union(i, geom_Rect_CenterAxisRatio(frg, float2(200, 300), float2(40, 10), 0.5f));
        i = aa_union(i, geom_Box_MinMax(frg, float2(100, 300), float2(150, 320)));
        i = aa_union(i, geom_EllipticDisc_CenterHalfAxisRatio(frg, float2(350, 100), float2(0,50.f), .2f));
        #endif
        texCol.rgb = i.value;
    }
#endif

    texCol.rgb = pow(saturate(texCol.rgb), 2.2);
    //texCol.a = 1;

    OUT.Col = texCol;
}
