
struct v2p {
    float4 Pos : SV_Position;
};

struct p2f {
    float4 Col : SV_Target0;
};

void pmain(in v2p IN, out p2f OUT)
{
    OUT.Col = float4(1,0,1,1);
}
