
struct a2v {
    float2 Pos       : POSITION;
    float4 Tex       : TEXTURE;
    float4 StrokeCol : COLOR0;
    float4 FillCol   : COLOR1;
};

struct v2p {
    float4 Pos       : SV_Position;
    float4 Tex       : TEXTURE;
    float4 StrokeCol : COLOR0;
    float4 FillCol   : COLOR1;
};

uniform float2 viewport_resolution;

void vmain(in a2v IN, out v2p OUT)
{
    OUT.Pos.xy = (IN.Pos.xy / viewport_resolution) * float2(2,-2) - float2(1,-1);
    OUT.Pos.z = 0.5f;
    OUT.Pos.w = 1.f;
    OUT.Tex = IN.Tex;
    OUT.StrokeCol = IN.StrokeCol;
    OUT.FillCol = IN.FillCol;
    //OUT.FillCol = float4(0,0,1,1);
}
