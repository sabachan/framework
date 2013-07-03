
Texture2D<float4> texture_earthGolem;
SamplerState sampler_input;

uniform uint date_in_frames;

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

    // Note: As we know that render target resolution is an integer, we can
    // compute its exact value by rounding approximation using ddx(texCoords):
    uint rtw = 1.f/ddx(IN.Tex.x) + 0.5f;
    uint rth = 1.f/ddy(IN.Tex.y) + 0.5f;
    float rtratio = rtw/rth;
    float2 rtwh = float2(rtw,rth);
    float2 rtpos = IN.Tex.xy * rtwh;

    uint texW, texH, texLevelCount;
    texture_earthGolem.GetDimensions(0, texW, texH, texLevelCount);
    uint2 texWH = uint2(texW, texH);

    float2 mvtCenter = 0.5f * rtwh;
    float radius = 0.2f * rth;
    uint frameIndexUnbounded = date_in_frames / 12;
    uint frameIndex = frameIndexUnbounded % 12;
    uint framesPerTurn = uint(2*radius);
    float turnFactor = frac( frameIndexUnbounded * 1.f/framesPerTurn );
    float alpha = -turnFactor *2*3.14159265359;
    float cosAlpha = cos(alpha);
    float sinAlpha = sin(alpha);
    float2 spritePos = mvtCenter + radius * float2(cosAlpha, sinAlpha);
    int2 spriteWHinWorld = 2* int2(64,64);
    int2 spriteOffset = -2* int2(32,56);
    int2 spriteTLinWorld = int2(spritePos + spriteOffset);
    uint orientation = 0;
    if(cosAlpha > 0.9)
        orientation = 3;
    else if(cosAlpha < -0.9)
        orientation = 0;
    else if(cosAlpha > 0)
    {
        if(sinAlpha > 0)
            orientation = 2;
        else
            orientation = 4;
    }
    else
    {
        if(sinAlpha > 0)
            orientation = 1;
        else
            orientation = 5;
    }
    //orientation = (date_in_frames/100) % 6;
    //orientation = 0;
    //frameIndex = 1;
    //uint frameConversion[12] = { 1,0,3,2,4,6,5,7,8,9,10,11};
    //frameIndex = frameConversion[frameIndex];
    uint2 spriteWHinTex = uint2(64,64);
    uint2 spriteTLinTex = spriteWHinTex * uint2(orientation, frameIndex);

    float4 texCol = 0;
    int2 posInSprite = uint2(rtpos-spriteTLinWorld) * spriteWHinTex / spriteWHinWorld;
    if(posInSprite.x < 64 && posInSprite.y < 64)
        texCol = texture_earthGolem.Sample(sampler_input, (posInSprite + spriteTLinTex - float2(0.5,0.5)) / float2(texWH));

    texCol.rgb *= texCol.a;

    //uint texW, texH, texLevelCount;
    //texture_earthGolem.GetDimensions(0, texW, texH, texLevelCount);
    //float texRatio = float(texW)/texH;
    //float4 texCol = texture_earthGolem.Sample(sampler_input, (IN.Tex.xy-float2(0.5,0.5)) * float2(ratio/texRatio, 1) + float2(0.5,0.5));

    //texCol += 0*texture_input.Sample(sampler_input1, IN.Tex.xy * float2(ratio, 1) * 2 + 0.5);
    //texCol += 0*texture_input.Sample(sampler_input2, IN.Tex.xy * float2(ratio, 1) * 2 + 0.5);

    //texCol.rgb = pow(abs(texCol.rgb), 2.2f);
    OUT.Col = texCol;
}
