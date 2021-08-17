cbuffer cbPerObject : register(b0)
{
    float4x4 World;
}

cbuffer cbPerFrame : register(b1)
{
    float4x4 View;
    float4x4 Proj;
    float3 Eye;
}

cbuffer cbLight : register(b2)
{
    float3 ModelColor;
    //float3 LightColor;
    //float3 LightDirection;
    //float SpecularPower;
}

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
    
    vOut.Norm = mul(vIn.Norm, (float3x3)World);
    vOut.Tang = mul(vIn.Tang, (float3x3)World);
    vOut.TexC = vIn.TexC;
    
    return vOut;
}

struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular; // w = SpecPower
    float4 Reflect;
};

struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float  pad;
};

void ComputeDirectionalLight(
    Material mat, DirectionalLight L,
    float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec )
{
	// Initialize outputs.
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;

	// Add ambient term.
    ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	
    float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
}

float4 PS(PS_DATA pIn) : SV_TARGET
{
    
    //float3 mat_ambient = float3(.1, .1, .1);
    ////float3 mat_diffuse = float3(1, 0, 0);
    //float3 mat_diffuse = ModelColor;
    
    float3 light_color = float3(1, 1, 1);
    ////float3 light_color = LightColor;
    
    //float3 light_dir = float3(0, 0, 1);
    ////float3 light_dir = LightDirection;
    
    //float3 light_strengh = float3(1, 1, 1);
    
    //float spec_power = .2;
    //float spec_power = SpecularPower;
    
    //*********
    
    DirectionalLight light;
    light.Ambient = float4(.1, .1, .1, 1.0);
    light.Diffuse = float4(light_color, 1.0);
    light.Specular = float4(.2, .2, .2, 1.);
    light.Direction = float3(0, 0, 1);
    
    Material mat;
    mat.Ambient = float4(.1, .1, .1, 1.0);
    mat.Diffuse = float4(ModelColor, 1.0);
    mat.Specular = float4(.5, .5, .5, 256.);
    mat.Reflect = float4(.9, .9, .9, 1.);
    
    // Interpolating normal can unnormalize it, so normalize it.
    pIn.Norm = normalize(pIn.Norm);

    float3 toEyeW = normalize(Eye - pIn.PosW);

	// Start with a sum of zero. 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
    float4 A, D, S;

    ComputeDirectionalLight(mat, light, pIn.Norm, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    
    float4 litColor = ambient + diffuse + spec;

	// Common to take alpha from diffuse material.
    litColor.a = mat.Diffuse.a;

    return litColor;
    
}

//technique11 Main
//{
//    pass p0
//    {
//        SetVertexShader(CompileShader(vs_5_0, VS()));
//        SetHullShader(NULL);
//        SetDomainShader(NULL);
//        SetGeometryShader(NULL);
//        SetPixelShader(CompileShader(ps_5_0, PS()));
//    }
//}