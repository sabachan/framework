
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
    uint2 WH;
    uint mipCount;
    texture_input.GetDimensions(0, WH.x, WH.y, mipCount);

    float2 uv = IN.Tex.xy;
    float2 P = uv * WH;
    float dP = length(ddx(P));
    float2x2 dPdxy = { ddx(P).x, ddx(P).y, ddy(P).x, ddy(P).y };

    int3 loadpos;
    loadpos.xy = floor(P-0.5f);
    loadpos.z = 0;

    float4 tex00 = texture_input.Load(loadpos);
    //float4 tex10 = texture_input.Load(loadpos, int2(1,0));
    //float4 tex01 = texture_input.Load(loadpos, int2(0,1));
    //float4 tex11 = texture_input.Load(loadpos, int2(1,1));
    float4 tex10 = texture_input.Load(loadpos + int3(1,0,0));
    float4 tex01 = texture_input.Load(loadpos + int3(0,1,0));
    float4 tex11 = texture_input.Load(loadpos + int3(1,1,0));

    float smoothness = 0.5f;
    // NB: Assumes uniform scale of texture. TODO: support for anisotropy
    float2 t = smoothstep(0.5f - smoothness * dP, 0.5f + smoothness * dP, P-0.5f - loadpos.xy);

    float4 tex = lerp(lerp(tex00, tex10, t.x), lerp(tex01, tex11, t.x), t.y);
    float4 col = tex;
    OUT.Col = col;
}

