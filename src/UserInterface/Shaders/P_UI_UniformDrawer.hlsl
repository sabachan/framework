
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
    float4 inCol = IN.Col;
#if defined(APPLY_PREMULTIPLICATION) && APPLY_PREMULTIPLICATION
    inCol.rgb *= inCol.a;
#endif

    OUT.Col.rg = IN.Tex.xy;
    OUT.Col.b = 0;
    OUT.Col.a = 1;

    float w = 1.f/ddx(IN.Tex.x);
    float h = 1.f/ddy(IN.Tex.y);
    float ratio = w/h;
    float2 wh = float2(w,h);

    OUT.Col = inCol;
}
