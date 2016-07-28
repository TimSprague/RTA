#pragma pack_matrix(row_major)

struct V_IN
{
	float4 posL : POSITION;
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float4 posW : WORLD_POS;
};

cbuffer OBJECT : register(b0)
{
	float4x4 worldMatrix;
}

cbuffer SCENE : register(b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

V_OUT main(V_IN input2)
{
	V_OUT output = (V_OUT)0;

	float4 localH = float4(input2.posL.xyz, 1);

	output.posW = localH;

	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);

	output.posH = localH;

	return output;
}