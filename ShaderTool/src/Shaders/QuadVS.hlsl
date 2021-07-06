cbuffer cbPerObject : register(b0)
{
    float4x4 World;
}

cbuffer cbPerFrame : register(b1)
{
    float4x4 View;
    float4x4 Proj;
}

struct VSInput
{
    float3 pos : POSITION;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
};

VSOutput main(VSInput vIn)
{
    VSOutput vOut;
    
    float4 posW = mul(float4(vIn.pos, 1.0), World);
    vOut.PosH = mul(posW, View);
    vOut.PosH = mul(vOut.PosH, Proj);
    
    return vOut;
}