texture2D baseTexture : register(t0);

SamplerState filters : register(s0);

struct P_IN
{
	float4 posH : SV_POSITION;
	float2 uv : UV;
	float4 normal : NORMALS;
	float4 worldPos : WORLD_POS;
};

float4 main(P_IN input) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filters, input.uv.xy);
	return baseColor;
}