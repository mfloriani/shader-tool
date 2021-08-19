cbuffer cbPerObject : register(b0)
{
    float4x4 World;
}

cbuffer cbPerFrame : register(b1)
{
    float4x4 View;
    float4x4 Proj;
}

Texture2D tex2d : register(t0);
SamplerState samplerState : register(s0);

struct VS_DATA
{
    float3 Pos : POSITION;
    float3 Norm : NORMAL;
    float3 Tang : TANGENT;
    float2 TexC : TEXCOORD;
};

struct PS_DATA
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITIONT;
    float3 Norm : NORMAL;
    float3 Tang : TANGENT;
    float2 TexC : TEXCOORD;
};

PS_DATA VS(VS_DATA vIn)
{
    PS_DATA vOut;
    
    float4 posW = mul(float4(vIn.Pos, 1.0), World);
    vOut.PosW = posW.xyz;
    
    vOut.PosH = mul(posW, View);
    vOut.PosH = mul(vOut.PosH, Proj);
    
    vOut.Norm = mul(vIn.Norm, (float3x3) World);
    vOut.Tang = mul(vIn.Tang, (float3x3) World);
    vOut.TexC = vIn.TexC;
    
    return vOut;
}

float4 PS(PS_DATA pIn) : SV_TARGET
{
    return tex2d.Sample(samplerState, pIn.TexC);
}
