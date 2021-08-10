cbuffer cbPerObject : register(b0)
{
    float4x4 World;
}

cbuffer cbPerFrame : register(b1)
{
    float4x4 View;
    float4x4 Proj;
    float3   Eye;
}

cbuffer cbLight : register(b2)
{
    float3 Color;
    float3 LightDirection;
    float  SpecularPower;
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
    float4 PosH    : SV_POSITION;
    float3 Norm    : NORMAL;
    float3 Tang    : TANGENT;
    float2 TexC    : TEXCOORD;
    float3 Color   : COLOR;
    float3 ViewDir : TEXCOORD1;
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
    
    vOut.ViewDir = Eye - posW.xyz;
    
	return vOut;
}

float4 PS(PS_DATA pIn) : SV_TARGET
{
    float3 L = normalize(LightDirection);
    float3 N = normalize(pIn.Norm);
    float3 V = normalize(pIn.ViewDir);
    
    float3 R = reflect(-L, N);
    
    float3 diffuse = Color * saturate(dot(L, N));
    float3 specular = float3(1., 1., 1.) * pow(saturate(dot(R, V)), SpecularPower);
    
    return float4(diffuse + specular, 1.0);
}