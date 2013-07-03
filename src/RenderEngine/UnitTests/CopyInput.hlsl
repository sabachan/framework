
Texture2D<float4> texture_input;
SamplerState sampler_input;
SamplerState sampler_input1;
SamplerState sampler_input2;

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
};

struct p2f {
    float4 Col : SV_Target0;
};

void pmain(in v2p IN, out p2f OUT)
{
    OUT.Col.rg = IN.Tex.xy;
    OUT.Col.b = 0;
    OUT.Col.a = 1;

    float w = 1.f/ddx(IN.Tex.x);
    float h = 1.f/ddy(IN.Tex.y);
    float ratio = w/h;
    float2 wh = float2(w,h);

    uint texW, texH, texLevelCount;
    texture_input.GetDimensions(0, texW, texH, texLevelCount);
    float texRatio = float(texW)/texH;
    float4 texCol = texture_input.Sample(sampler_input, (IN.Tex.xy-float2(0.5,0.5)) * float2(ratio/texRatio, 1) + float2(0.5,0.5));

    //texCol += 0*texture_input.Sample(sampler_input1, IN.Tex.xy * float2(ratio, 1) * 2 + 0.5);
    //texCol += 0*texture_input.Sample(sampler_input2, IN.Tex.xy * float2(ratio, 1) * 2 + 0.5);
    
    OUT.Col = texCol;
}
