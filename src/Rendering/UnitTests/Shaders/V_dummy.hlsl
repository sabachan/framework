struct a2v {
    float2 Pos : POSITION;
};

struct v2p {
    float4 Pos : SV_Position;
};

void vmain(in a2v IN, out v2p OUT)
{
    OUT.Pos = float4(0,0,0,0);
}
