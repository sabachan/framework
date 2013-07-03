
Texture2D<float4> texture_input;
SamplerState sampler_input;

struct v2p {
    float4 Pos : SV_Position;
    float2 Tex : TEXTURE;
};

struct p2f {
    float4 Col : SV_Target0;
};

void pmain(in v2p IN, out p2f OUT)
{
    float4 col = texture_input.Sample(sampler_input, IN.Tex.xy);

    if(0) // little ad hoc blur
    {
        float ex = 0.65f * ddx(IN.Tex.x);
        float ey = 0.65f * ddy(IN.Tex.y);
        col = texture_input.Sample(sampler_input, IN.Tex.xy + float2(-ex, -ey));
        col += texture_input.Sample(sampler_input, IN.Tex.xy + float2(ex, -ey));
        col += texture_input.Sample(sampler_input, IN.Tex.xy + float2(-ex, ey));
        col += texture_input.Sample(sampler_input, IN.Tex.xy + float2(ex, ey));
        col /= 4;
    }
    //col *= float4(1,0.5,0.9,1);
    //col.a = 1;
    OUT.Col = col;
}