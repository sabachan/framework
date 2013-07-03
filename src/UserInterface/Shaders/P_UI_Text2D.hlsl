
Texture2D<float> texture_glyph_atlas;
SamplerState sampler_input;

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
    uint4 Col : COLOR0;
    uint4 ColStroke : COLOR1;
    uint4 Box : COLOR2;
    uint4 QuadData2 : COLOR3;
};

struct p2f {
    float4 Col : SV_Target0;
};

void pmain(in v2p IN, out p2f OUT)
{
    OUT.Col.rg = IN.Tex.xy;
    OUT.Col.b = 0;
    OUT.Col.a = 1;
    
    uint texW, texH, texLevelCount;
    texture_glyph_atlas.GetDimensions(0, texW, texH, texLevelCount);
    float ootexW = 1.f / texW;
    float ootexH = 1.f / texH;
    
    float4 uvbox = (IN.Box + float4(-0.5f,-0.5f,0.5f,0.5f)) * float4(ootexW, ootexH, ootexW, ootexH);
    float2 uv = IN.Tex.xy;
    uv = clamp(uv, uvbox.xy, uvbox.zw);

    float fillIntensity = texture_glyph_atlas.Sample(sampler_input, uv);
    float strokeIntensity = fillIntensity;
#if 0
    int R = 1;
    for(int i = -R; i <= R; ++i)
        for(int j = -R; j <= R; ++j)
        {
            if(i != 0 || j != 0)
                strokeIntensity = max(strokeIntensity, texture_glyph_atlas.Sample(sampler_input, clamp(IN.Tex.xy + float2(i * ootexW, j * ootexH), uvbox.xy, uvbox.zw)));
        }
#endif
    //texCol += 0*texture_input.Sample(sampler_input1, IN.Tex.xy * float2(ratio, 1) * 2 + 0.5);
    //texCol += 0*texture_input.Sample(sampler_input2, IN.Tex.xy * float2(ratio, 1) * 2 + 0.5);
    
    float4 col = float4(0,0,0,0);
    //col = lerp(col, IN.ColStroke / 255.f, strokeIntensity);
    col = lerp(col, IN.Col / 255.f, fillIntensity);
    //col = uv.x == IN.Tex.x ? float4(0.1,0.1,0.1,0) : 0;
    //col = 1;
    OUT.Col = col;
}
