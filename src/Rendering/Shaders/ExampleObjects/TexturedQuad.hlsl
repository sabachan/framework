#include "../../../Shaders/Geometry.hlsl"
#include "../../../Shaders/CirclesFont.hlsl"
#include "../../../Shaders/ProceduralTexture.hlsl"

Texture2D<float4> texture_input;
SamplerState sampler_input;

//uniform uint date_in_frames;

struct a2v {
    float2 Pos : POSITION;
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
    OUT.Pos.xy = IN.Pos;
    OUT.Pos.z = 1.f-1.e-5f;
    OUT.Pos.w = 1.f;
    OUT.Tex = IN.Tex;
}

void pmain(in v2p IN, out p2f OUT)
{
    OUT.Col.rg = IN.Tex.xy;
    OUT.Col.b = 0;
    OUT.Col.a = 1;

    float w = 1.f/ddx(IN.Tex.x);
    float h = 1.f/ddy(IN.Tex.y);
    float ratio = w/h;

    float4 texCol = texture_input.Sample(sampler_input, IN.Tex.xy * float2(ratio, 1) * 2 + 0.5);
    /*
    int3 pos;
    pos.xy = IN.Pos.xy;
    pos.z = 0;
    float4 texCol = texture_input.Load(pos);
    */
#if 0
    {
        aa_Fragment frg = aa_Create2DFragmentFromPos((IN.Tex.xy * 2 - 1) * float2(1,-1));
        frg.aaradius = 1;
        aa_Intensity i = geom_HalfPlane_PointNormal(frg, float2(0.1,0), float2(-100,-400));
        i = aa_xor(i, geom_Line_PointNormal(frg, float2(0.1,0), float2(-1,4), 0.02f));
        texCol.rgb = i.value;
    }
#endif
#if 0
    {
        aa_Fragment frg = aa_Create2DFragmentFromPos(IN.Pos.xy);
        frg.aaradius = 2.001;
        frg.aaradius = 1.999;
        frg.aaradius = 1;
        aa_Intensity i = aa_CreateIntensity(0);
        i = aa_union(i, geom_HalfPlane_PointNormal(frg, float2(500,0), float2(-100,400)));
        i = aa_union(i, geom_Line_PointNormal(frg, float2(0,300), float2(1,4), 1.f));
        i = aa_union(i, geom_Disc(frg, float2(250, 100), 50));
        i = aa_union(i, geom_Circle(frg, float2(100, 100), 50, 1.f));
        i = aa_union(i, geom_Parallelogram_CenterEdges(frg, float2(300, 300), float2(40, 10), float2(40, 30)));
        i = aa_union(i, geom_Rect_CenterAxisRatio(frg, float2(200, 300), float2(40, 10), 0.5f));
        i = aa_union(i, geom_Box_MinMax(frg, float2(100, 300), float2(150, 320)));
        i = aa_union(i, geom_EllipticDisc_CenterHalfAxisRatio(frg, float2(350, 100), float2(0,50.f), .2f));
        texCol.rgb = i.value;
    }
#endif
#if 0
    {
        circlesfont_Parameters fontParam = circlesfont_CreateParameters(30);
        {
            circlesfont_Parameters p;
            p.size = 20;
            p.ascend = 1.f;
            p.descend = 0.5f;
            p.xheight = 0.5f; // should be ~ 0.5
            p.interchar = 0.1f; //1.f - p.xheight;
            p.interline = 0.1f; //2.5f - p.descend - p.ascend;
            p.thickness = 0.03f;
            p.thinness = 0.03f;
            p.oblique = 0.5f;
            p.showdot = true;
            p.italic_inclination = 0.4f;
            p.dotpos = 1.5f * p.xheight;
            p = circlesfont_ComputeParameters(p);
            //p = circlesfont_ComputeHintingAndParameters(p);
            circlesfont_Validate(p);
            fontParam = p;
        }

        //fontParam = circlesfont_CreateParameters_Ref(14);
        //fontParam = circlesfont_CreateParameters_Ref(50);
        aa_Fragment frg = aa_Create2DFragmentFromPos(IN.Pos.xy);
        frg.aaradius = 1.f;
        aa_Intensity i = aa_CreateIntensity(0);
        //i = aa_union(i, circlesfont_DrawTests(frg, fontParam, float2(50.5f,120.5f)));
        i = aa_union(i, circlesfont_DrawTests(frg, fontParam, float2(30,60)));
        texCol.rgb = i.value;
        if(!fontParam.isValidated && 1)
            texCol.gb = 0;
    }
#endif
    //texCol.rgb = pow(saturate(texCol.rgb), 2.2);

#if 0
    {
        aa_Fragment frg = aa_Create2DFragmentFromPos(IN.Pos.xy);
        frg = mul(frg, float2(1, -1));
        frg = add(frg, float2(-0.5f/ddx(IN.Tex.x), 0.5f/ddy(IN.Tex.y)));
        frg.aaradius = 1;
        texCol = ptex_SimplexTest(frg.pos);
        texCol.a = 1;
    }
#endif

#if 1
    {
        aa_Intensity intensity = aa_CreateIntensity(0);
        if(0)
        {
        circlesfont_Parameters fontParam = circlesfont_CreateParameters(30);
        {
            circlesfont_Parameters p;
            p.size = 75;
            p.ascend = 1.f;
            p.descend = 0.5f;
            p.xheight = 0.5f; // should be ~ 0.5
            p.interchar = 0.1f; //1.f - p.xheight;
            p.interline = 0.1f; //2.5f - p.descend - p.ascend;
            p.thickness = 0.12f;
            p.thinness = 0.12f;
            p.oblique = 0.5f;
            p.showdot = true;
            p.italic_inclination = 0.f;
            p.dotpos = 1.5f * p.xheight;
            p = circlesfont_ComputeParameters(p);
            //p = circlesfont_ComputeHintingAndParameters(p);
            circlesfont_Validate(p);
            fontParam = p;
        }
        float2 P = float2(50,250);
        aa_Fragment frg = aa_Create2DFragmentFromPos(IN.Pos.xy);
        frg.aaradius = 15.f;
        frg = add(frg, -P);
        frg = mul(frg, 1.f/fontParam.size);
        fontParam.size = 1.f;
        float2 pos = float2(0,0);
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), H_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), e_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), l_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), l_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), o_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), UNICODE_SPACE)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), w_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), o_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), r_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), l_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), d_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, fontParam, pos * float2(fontParam.advance, fontParam.lineheight), UNICODE_EXCLAMATION_MARK)); pos.x += 1;
        }

        const uint seed = 562;
        RandomGenerator rand = rand_CreateGenerator(seed);
        rand.method = rand_Method_Hashing;

        float noise = 1;
        {
            float i = 0;
            float2 npos = 0.010325f * IN.Pos.xy + float2(100,255);
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
            float2 npos =0.01f * IN.Pos.xy;
            i += abs(perlin_SimplexGradientNoise(rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            i += abs(1.f/2.f * perlin_SimplexGradientNoise(2.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            //i += abs(1.f/4.f * perlin_SimplexGradientNoise(4.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            //i += abs(1.f/8.f * perlin_SimplexGradientNoise(8.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            //i += abs(1.f/16.f * perlin_SimplexGradientNoise(16.f * rand_GetUniform(rand, 0.9f, 1.15f) * float2(npos.x, npos.y) + rand_GetUniform(rand,float2(-1.f, -1.f), float2(1.f, 1.f)) * 100.f));
            noise *= 0.7f;
            noise += 0.3f * i;
            //noise += 0.5f * i;
        }

        float f = intensity.value;

        if(true)
        {
            float2 C = float2(300,250);
            float2 CP = IN.Pos.xy * float2(1.f, 1.f) - C;
            float d = sqrt(dot(CP,CP));
            f = saturate(1.5 - d / 150);
        }

        f *= 0.7f;
        f += 0.3f * noise;
        //f = noise;

        //f = smoothstep(0.4f, 1.f, f);

        //if(f < 0.6f)
        //    f = 0;
        //else
        //    f = 1;

        texCol.rgb = f;
        texCol.a = 1;
        texCol = pow(abs(texCol), 2.2f);

        if (true)
        {
            texCol = lerp(float4(0,0,0,1), float4(0.5,0.2f,0.1f,1), smoothstep(0.4f, 0.7f, f));
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
            float a = 0.5f;
            float f = 10.5f;
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
