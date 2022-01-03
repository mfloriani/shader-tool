#define IDENTITY_MATRIX float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)

static float4x4 invView = IDENTITY_MATRIX;

cbuffer PerFrameCB : register(b0)
{
    float time;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float2 canvasXY : TEXCOORD0;
};

VS_Output VS(float4 pos : POSITION)
{
    VS_Output output;
    output.position = float4(sign(pos.xy), 0, 1);

    float aspectRatio = IDENTITY_MATRIX._m00 / IDENTITY_MATRIX._m11;
    output.canvasXY = sign(pos.xy) * float2(1.0, aspectRatio);

    return output;
}

float4x4 inverse(float4x4 m)
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

float4 PS(VS_Output input) : SV_TARGET
{
    float3 ro = mul(float4(0, 0, 0, 1.0f), invView).xyz;
    float3 pixelPos = float3(input.canvasXY, -1);
    float3 rd = normalize(mul(float4(pixelPos, 0.0f), invView)).xyz;
    
    float t = time * .1 + ((.25 + .05 * sin(time * .1)) / (length(input.canvasXY) + .07)) * 2.2;
    float si = sin(t);
    float co = cos(t);
    float2x2 ma = float2x2(co, si, -si, co);

    float v1, v2, v3;
    v1 = v2 = v3 = 0.0;
	
    float s = 0.0;
    for (int i = 0; i < 90; i++)
    {
        float3 p = s * float3(input.canvasXY, 0.0);
        p.xy = mul(ma, p.xy);
        p += float3(.22, .3, s - 1.5 - sin(time * .13) * .1);
        for (int i = 0; i < 8; i++)
            p = abs(p) / dot(p, p) - 0.659;
        v1 += dot(p, p) * .0015 * (1.8 + sin(length(input.canvasXY * 13.0) + .5 - time * .2));
        v2 += dot(p, p) * .0013 * (1.5 + sin(length(input.canvasXY * 14.5) + 1.2 - time * .3));
        v3 += length(p.xy * 10.) * .0003;
        s += .035;
    }
	
    float len = length(input.canvasXY);
    v1 *= smoothstep(.7, .0, len);
    v2 *= smoothstep(.5, .0, len);
    v3 *= smoothstep(.9, .0, len);
	
    float3 col = float3(v3 * (1.5 + sin(time * .2) * .4),
					(v1 + v3) * .3,
					 v2) + smoothstep(0.2, .0, len) * .85 + smoothstep(.0, .6, v3) * .3;

    return float4(min(pow(abs(col), (float3) 1.2), 1.0), 1.0);
}