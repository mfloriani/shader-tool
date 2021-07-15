Texture2D renderTex : register(t0);

SamplerState samPointWrap : register(s0);
//SamplerState samPointClamp : register(s1);
//SamplerState samLinearWrap : register(s2);
//SamplerState samLinearClamp : register(s3);
//SamplerState samAnisotropicWrap : register(s4);
//SamplerState samAnisotropicClamp : register(s5);

struct PSInput
{
    float4 PosH : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tang : TANGENT;
    float2 TexC : TEXCOORD;
};

float4 main(PSInput pIn) : SV_TARGET
{
    return renderTex.Sample(samPointWrap, pIn.TexC);
}