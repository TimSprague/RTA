#pragma pack_matrix(row_major)

struct V_IN
{
	float3 posL : POSITION;
	float4 col : COLOR;
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float4 col : COLOR;
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

cbuffer LINES : register(b3)
{
	float4x4 array[36];
}

V_OUT main(V_IN input1, uint id:SV_InstanceID)
{
	V_OUT output = (V_OUT)0;

	float4 localH = float4(input1.posL.xyz, 1);

	output.worldPos = localH.xyzw;

	localH = mul(localH, array[id]);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);

	output.posH = localH;
	output.col = input1.col;

	return output;
}