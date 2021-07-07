cbuffer cbPerObject : register(b0)
{
    float4x4 World;
}

cbuffer cbPerFrame : register(b1)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 RTProj;
}

struct VSInput
{
    float3 Pos  : POSITION;
    float3 Norm : NORMAL;
    float2 TexC : TEXCOORD;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float3 Norm : NORMAL;
    float2 TexC : TEXCOORD;
};

VSOutput main( VSInput vIn )
{
    VSOutput vOut;
    
    float4 posW = mul(float4(vIn.Pos, 1.0), World);
    vOut.PosH = mul(posW, View);
    //vOut.PosH = mul(vOut.PosH, Proj);
    vOut.PosH = mul(vOut.PosH, RTProj);
    
    vOut.Norm = mul(float4(vIn.Norm, 1.0), World);
    
    vOut.TexC = vIn.TexC;
    
	return vOut;
}