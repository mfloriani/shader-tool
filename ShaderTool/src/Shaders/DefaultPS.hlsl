struct PSInput
{
    float4 PosH : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tang : TANGENT;
    float2 TexC : TEXCOORD;
    float3 Color : COLOR;
};

float4 main(PSInput pIn) : SV_TARGET
{
    return float4(pIn.Color, 1.0);
}