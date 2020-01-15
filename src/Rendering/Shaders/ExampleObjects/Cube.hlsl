#include "../../../Shaders/Geometry.hlsl"
#include "../../../Shaders/CirclesFont.hlsl"

struct a2v { 
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float4 Col : COLOR;
    float2 Tex : TEXTURE;
};

struct v2p {
    float4 Pos : SV_Position;
    float4 Normal : NORMAL;
    float4 Col : COLOR;
    float2 Tex : TEXTURE;
};

struct p2f {
    float4 Col : SV_Target0;
};

uniform float4x4 camera_matrix;

void vmain(in a2v IN, out v2p OUT) 
{
    float4 pos;
    pos.xyz = IN.Pos;
    pos.w = 1;

    float rx = 3.14f/3.f;
    float cx = cos(rx);
    float sx = sin(rx);
    float4x4 mx = {
        1,  0,  0,  0,
        0, cx, sx,  0,
        0,-sx, cx,  0,
        0,  0,  0,  1,
    };
    float rz = 3.14f/4.f;
    float cz = cos(rz);
    float sz = sin(rz);
    float4x4 mz = {
         cz, sz, 0, 0,
        -sz, cz, 0, 0,
          0,  0, 1, 0,
          0,  0, 0, 1,
    };
    float4x4 mt = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 3,
        0, 0, 0, 1,
    };
    float4x4 id = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    float4x4 cam = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 1, 0,
    };
    
    // This matrix is used to output vertices in an indirect frame (DirectX)
    float4x4 directToIndirect = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0,-1, 0,
        0, 0, 0, 1,
    };

    //float4x4 m = mul(mt, mul(mx, mz));
    //float4x4 m = mt;
    float4x4 m = id;
    //float4x4 m = {
    //    1, 0, 0, 0,
    //    0,-1, 0, 0,
    //    0, 0, 1, 3,
    //    0, 0, 1, 3,
    //};
    //float4x4 m = {
    //    1, 0, 0, 0,
    //    0,-1, 0, 0,
    //    0, 0, 1, 1,
    //    0, 0, 3, 3,
    //};

    //pos = mul(m, pos);
    //pos = mul(directToIndirect, pos);
    //pos = mul(cam, pos);
    pos = mul(camera_matrix, pos);

    //m = mul(camera_matrix, m);
    //pos = mul(m, pos);

    OUT.Pos = pos;
    float4 normal;
    normal.xyz = IN.Normal;
    normal.w = 0.f;
    OUT.Normal = mul(m, normal); // assuming m is a rigid transform (no scale, no skew).
    OUT.Col = IN.Col;
    OUT.Tex = IN.Tex;
}

void pmain(in v2p IN, out p2f OUT)
{
    float3 ambiant = 2*float3(0.15f,0.1f,0.1f);
    float3 lightColor = 0.9* float3(0.95f,1.f,0.8f);
    float3 lightDir = normalize(float3(-1.f,-0.5f,0.3f));
    //float3 ambiant = 2*float3(0.15f,0.1f,0.4f);
    //float3 lightColor = 0.9* float3(0.95f,1.f,0.8f);
    //float3 lightDir = normalize(float3(0.3f,1.f,1.5f));
    float3 light = lightColor * saturate( dot(IN.Normal.xyz, lightDir) );
    
    float4 col;
    col.rgb = (ambiant + light) * IN.Col.rgb;
    col.rgb = (ambiant + light) * float3(1,1,1);
    //col.rgb = IN.Col.rgb;
    col.a = IN.Col.a;

    if(IN.Pos.z < 0.0001f) col.rgb = 0;
    OUT.Col = col;
    //OUT.Col.rgb = IN.Pos.z;

    //OUT.Col.rgb = 0.002;
    //OUT.Col.a = 0.;

    //return;

#if 1
    {
        circlesfont_Parameters p;
        p.size = 0.3f;
        p.ascend = 1.f;
        p.descend = 0.5f;
        p.xheight = 0.5f; // should be ~ 0.5
        p.interchar = 0.05f; //1.f - p.xheight;
        p.interline = 0.f; //2.5f - p.descend - p.ascend;
        p.thickness = 0.1f;
        p.thinness = 0.1f;
        p.oblique = 0.5f;
        p.showdot = true;
        p.italic_inclination = 0.f;
        p.dotpos = 1.5f * p.xheight;
        p = circlesfont_ComputeParameters(p);
        //p = circlesfont_ComputeHintingAndParameters(p);
        //circlesfont_Validate(p);
        circlesfont_Parameters param = p;

        aa_Fragment frg = aa_Create2DFragmentFromPos(IN.Tex);
        //aa_Fragment frg = aa_Create2DFragmentFromPosRadius(IN.Tex, 0.001f);
        frg.aaradius = 1.f;
        aa_Intensity intensity = aa_CreateIntensity(0);

        //intensity == aa_union( intensity,  );

        float2 pos = float2(0, 0);
        pos.y += 1.f;  pos.x = 0.2f;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), H_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), e_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), l_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), l_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), o_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), UNICODE_SPACE)); pos.x += 1;
        pos.y += 1;  pos.x = 0.2f;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), w_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), o_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), r_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), l_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), d_)); pos.x += 1;
        intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * param.size*float2(param.advance, param.lineheight), UNICODE_EXCLAMATION_MARK)); pos.x += 1;

        //intensity = geom_Disc3Points(frg, float2(0.5f, 0.f), float2(0.f, 0.5f), float2(0.5f, 1.f));

        OUT.Col.rgb = 1-intensity.value;
        OUT.Col.rgb = (ambiant + light) * OUT.Col.rgb;
    }
#endif
 //OUT.Col = 0;
}
