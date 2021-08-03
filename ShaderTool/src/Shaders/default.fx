cbuffer cbPerObject : register(b0)
{
    float4x4 World;
}

cbuffer cbPerFrame : register(b1)
{
    float4x4 View;
    float4x4 Proj;
}

cbuffer cbColor : register(b2)
{
    float3 Color;
}

struct VS_DATA
{
    float3 Pos  : POSITION;
    float3 Norm : NORMAL;
    float3 Tang : TANGENT;
    float2 TexC : TEXCOORD;
};

struct PS_DATA
{
    float4 PosH : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tang : TANGENT;
    float2 TexC : TEXCOORD;
    float3 Color: COLOR;
};

PS_DATA VS(VS_DATA vIn)
{
    PS_DATA vOut;
    
    float4 posW = mul(float4(vIn.Pos, 1.0), World);
    vOut.PosH = mul(posW, View);
    vOut.PosH = mul(vOut.PosH, Proj);
    
    vOut.Norm = mul(float4(vIn.Norm, 1.0), World).xyz;
    vOut.Tang = mul(float4(vIn.Tang, 1.0), World).xyz;
    vOut.TexC = vIn.TexC;
    vOut.Color = Color;
    
	return vOut;
}

float4 PS(PS_DATA pIn) : SV_TARGET
{
    return float4(pIn.Color, 1.0);
}