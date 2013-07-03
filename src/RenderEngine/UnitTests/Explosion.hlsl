#include "../../Shaders/ProceduralTexture.hlsl"

//uniform uint date_in_frames;

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

    float4 texCol = float4(0,0,0,1);
#if 1
    {
        const uint seed = 562;
        RandomGenerator rand = rand_CreateGenerator(seed);
        rand.method = rand_Method_Hashing;

        float noise = 1;
        float2 pos = IN.Pos.xy;

        if(true)
        {
            float i = 0;
            float2 npos = 0.010325f * pos + float2(100,255);
            i += perlin_SimplexGradientNoise(rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f);
            i += 1.f/2.f * perlin_SimplexGradientNoise(2.0023f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f);
            i += 1.f/4.f * perlin_SimplexGradientNoise(4.1256f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f);
            i += 1.f/8.f * perlin_SimplexGradientNoise(8.3107f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f);
            i += 1.f/16.f * perlin_SimplexGradientNoise(16.2223f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f);
            //i += 1.f/32.f * perlin_SimplexGradientNoise(32.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1f), float2(-1.f, -1f)) * 100.f);
            noise = 0.5f + 0.25f * i;
        }
        //rand = rand_CreateGenerator(seed);
        //rand.method = rand_Method_Hashing;
        if(true)
        {
            float i = 0;
            float2 npos =0.01f * pos;
            // we use an offset so that the abs non-derivative does not show exactly at 0 of noise, which ca show straight lines 
            i += 0.8f*abs(0.3f+ perlin_SimplexGradientNoise(rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            i += 0.8f*abs(0.15f+ 1.f/2.f * perlin_SimplexGradientNoise(2.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            //i += abs(1.f/4.f * perlin_SimplexGradientNoise(4.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            //i += abs(1.f/8.f * perlin_SimplexGradientNoise(8.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            //i += abs(1.f/16.f * perlin_SimplexGradientNoise(16.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            noise *= 0.7f;
            noise += 0.3f * i;
            //noise += 0.5f * i;
        }

        float f = 0;

        if(true)
        {
            float2 wh = float2(1.f/ddx(IN.Tex.x), 1.f/ddy(IN.Tex.y));
            float2 C = 0.5f * wh; // float2(300,250);
            float2 CP = IN.Pos.xy * float2(1.f, 1.f) - C;
            float d = sqrt(dot(CP,CP));
            f = saturate(1.5 - d / (0.4f * wh.y));
        }

        f *= 0.7f;
        f += 0.3f * noise;

        //f = smoothstep(0.4f, 1.f, f);

        //if(f < 0.6f)
        //    f = 0;
        //else
        //    f = 1;

        texCol.rgb = f;
        texCol = pow(abs(texCol), 2.2f);

        if (true)
        {
            texCol = lerp(float4(0,0,0,0), float4(0.5,0.2f,0.1f,1), smoothstep(0.4f, 0.7f, f));
            texCol = lerp(texCol, float4(1,0.9f,0.5f,1), smoothstep(0.6f, 1.f, f));
            texCol = lerp(texCol, float4(0.8f,0.95f,1.f,1), smoothstep(0.8f, 1.f, f));
        }
        if (false)
        {
            texCol = lerp(float4(0,0,0,1), float4(0.1,0.2f,0.8f,1), smoothstep(0.4f, 0.7f, f));
            texCol = lerp(texCol, float4(0.3,0.9f,0.5f,1), smoothstep(0.45f, 1.f, f));
            texCol = lerp(texCol, float4(0.95f,0.8f,0.5f,1), smoothstep(0.8f, 1.f, f));
        }
        if(false)
        {
            // domain distortion (cool)
            float2 npos = 0.01f * IN.Pos.xy;
            float i = 0;
            i = perlin_SimplexGradientNoise(rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f);
            float a = 0.05f;
            float f = 5.f;
            float x = npos.x + perlin_SimplexGradientNoise(rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) * f) * a;
            float y = npos.y + perlin_SimplexGradientNoise(rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) * f) * a;
            i = perlin_SimplexGradientNoise(float2(x, y));

            texCol.rgb = i*0.5f+0.5;
        }
        //texCol.rgb = f;
        texCol = pow(abs(texCol), 2.2f);
    }
#endif

    OUT.Col = texCol;
}
