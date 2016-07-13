#pragma pack_matrix(row_major)

struct V_IN
{
	float4 posH : SV_POSITION;
	float4 uv : UV;
	float4 normal : NORMAL;
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float4 uv : UV;
	float4 normal : NORMAL;
	float4 worldPos : WORLD_POS;
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

V_OUT main(P_IN input) 
{
	V_OUT output = (V_OUT)0;

	float localH = float4(input.posL.xyz, 1);

	output.worldPos = localH.xyzw;

	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);

	output.posH = localH;
	output.uv = input.uv;
	output.normal = mul(float4(input.norma.xyz, 0), worldMatrix);

	return output;
}