

struct a2v { 
    float3 Pos : POSITION;
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
    OUT.Pos.xyz = IN.Pos;
    OUT.Pos.w = 1.f;
    OUT.Tex = IN.Tex;
}

void pmain(in v2p IN, out p2f OUT)
{
    OUT.Col.rg = IN.Tex.xy;
    OUT.Col.b = 0;
    OUT.Col.a = 1;
    
}
