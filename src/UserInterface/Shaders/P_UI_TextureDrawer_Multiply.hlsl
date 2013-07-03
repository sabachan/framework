
Texture2D<float4> texture_input;
SamplerState sampler_input;

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
    float4 Col : COLOR0;
};

struct p2f {
    float4 Col : SV_Target0;
};

void pmain(in v2p IN, out p2f OUT)
{
    OUT.Col.rg = IN.Tex.xy;
    OUT.Col.b = 0;
    OUT.Col.a = 1;

    float2 uv = IN.Tex.xy;

    float4 tex = texture_input.Sample(sampler_input, uv);
    float4 col = tex * float4(tex.a,tex.a,tex.a,1);
    float t = col.b;
    //col = t > 0.5f ? float4(1,1,1,1) : t > 0.001f ? float4(0.5,0.5,0.5,1) : float4(0,0,0,0) ;
    col *= IN.Col;
    //col.a = tex.a;
    //col = float4(1,1,1,1);
    //col *= tex.b;
    OUT.Col = col;
}
