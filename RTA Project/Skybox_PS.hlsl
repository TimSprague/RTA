struct P_IN
{
	float4 posH : SV_POSITION;
	float4 posW : WORLD_POS;
};

textureCUBE baseTexture : register(t0);

SamplerState filters : register(s0);

float4 main(P_IN input2) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filters, input2.posW.xyz);
	return baseColor;
}