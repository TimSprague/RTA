texture2D baseTexture : register(t0);

SamplerState filters : register(s0);

struct P_IN
{
	float4 posH : SV_POSITION;
	float2 uv : UV;
	float4 normal : NORMALS;
	float4 worldPos : WORLD_POS;
};

cbuffer dLight : register(b0)
{
	float4 directionalDir;
	float4 directionalColor;
}

float4 main(P_IN input) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filters, input.uv.xy);

	float4 ambient = baseColor * .10;

	float lightRatio = saturate(dot(-directionalDir.xyz, input.normal.xyz));

	float4 result = lightRatio * directionalColor * baseColor;

	return result;
}