#include "../../Shaders/AntiAliasing.hlsl"
#include "../../Shaders/Geometry.hlsl"


struct v2p {
    float4 Pos : SV_Position;
    float4 Tex : TEXTURE;
    float4 StrokeCol : COLOR0;
    float4 FillCol : COLOR1;
};

struct p2f {
    float4 Col : SV_Target0;
};

void pmain(in v2p IN, out p2f OUT)
{
    // In order to be able to draw Round line joints, we will use negative
    // values of stroke thickness to use a radius 0.
    // In order to choose between fill color inside or outside the circle,
    // we will use the w paramter.
    const bool fillOutsideMode = IN.Tex.w < 0;
    const float radius = IN.Tex.z < 0 ? 0 : 1;
    const float strokeThickness = abs(IN.Tex.z)*1.;
    const bool enableStroke = strokeThickness > 0;
    const float2 pos = IN.Tex.xy;
    aa_Fragment fragment = aa_Create2DFragmentFromPos(pos);
    fragment.aaradius = 1.5;

    float4 strokeCol = IN.StrokeCol;
    float4 extCol = float4(0,0,0,0);
    float4 fillCol = IN.FillCol;
    if(0) // show triangles
    {
        extCol = float4(0.2,0.1,0,0);
        strokeCol = float4(0.5,0.5,0.5,0);
        fillCol = float4(0.1,0.2,0.2,0);
    }

    const float4 outsideCol = fillOutsideMode ? fillCol : extCol;
    const float4 insideCol = fillOutsideMode ? extCol : fillCol;

#if 0
    aa_Intensity inIntensity = geom_Disc(fragment, float2(0,0), radius - 0.5f * strokeThickness);
    aa_Intensity outIntensity = geom_Disc(fragment, float2(0,0), radius + 0.5f * strokeThickness);

    float4 col = outsideCol;
    col = lerp(col, strokeCol, outIntensity.value);
    col = lerp(col, insideCol, inIntensity.value);
    OUT.Col = col;
#else
    aa_Intensity fillIntensity = geom_Disc(fragment, float2(0,0), radius);
    aa_Intensity strokeIntensity = geom_Circle(fragment, float2(0,0), radius, strokeThickness);

    float4 col = outsideCol;
    col = lerp(col, insideCol, fillIntensity.value);
    col = lerp(col, strokeCol, strokeIntensity.value);
    OUT.Col = col;
#endif
    if(0)
    {
        float w = 1.f/ddx(IN.Tex.x);
        float h = 1.f/ddy(IN.Tex.y);
        float ratio = w/h;
        float2 wh = float2(w,h);

        OUT.Col.rg = abs(IN.Tex.xy) * 1.f;
        OUT.Col.b = 0.05;
        OUT.Col.a = 0.5;
        //if(any(abs(IN.Tex.xy) > float2(0.5f, 0.5f) && abs(IN.Tex.xy) < float2(0.6f, 0.6f)))
        //    OUT.Col = float4(1,1,1,1);
        //if(any(abs(IN.Tex.xy) > float2(1.f, 1.f)))
        //    OUT.Col = float4(1,1,1,1);
        //OUT.Col = IN.Tex.z * 10000;
        //OUT.Col = IN.StrokeCol;
        //OUT.Col = IN.FillCol;
        //OUT.Col = IN.StrokeCol.w;
        //OUT.Col = IN.FillCol.x;
        //OUT.Col.a = 1;
        //OUT.Col = float4(1,0,0,1) * 0.1;
    }
}
