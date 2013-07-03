struct a2v {
    float2 Pos : POSITION;
    float2 Tex : TEXTURE;
};

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
};

void vmain(in a2v IN, out v2p OUT)
{
    OUT.Pos.xy = IN.Pos;
    OUT.Pos.z = 0.5f; //1.f-1.e-5f;
    OUT.Pos.w = 1.f;
    OUT.Tex = IN.Tex;
}
