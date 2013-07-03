
struct a2v {
    float2 Pos : POSITION;
    float2 Tex : TEXTURE;
    float4 Col : COLOR0;
};

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
    float4 Col : COLOR0;
};

uniform float2 viewport_resolution;

void vmain(in a2v IN, out v2p OUT)
{
    OUT.Pos.xy = (IN.Pos.xy / viewport_resolution) * float2(2,-2) - float2(1,-1);
    OUT.Pos.z = 0.5f; //1.f-1.e-5f;
    OUT.Pos.w = 1.f;
    OUT.Tex = IN.Tex;
    OUT.Col = IN.Col;
}
