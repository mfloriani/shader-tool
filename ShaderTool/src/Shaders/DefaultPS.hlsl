struct PSInput
{
    float4 PosH : SV_POSITION;
};

float4 main(PSInput pIn) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}