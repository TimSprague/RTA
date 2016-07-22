#pragma pack_matrix(row_major)

struct V_IN
{
	float3 posL : POSITION;
	float2 uv : UV;
	float3 normal : NORMALS;
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float2 uv : UV;
	float4 normal : NORMALS;
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

cbuffer CLONES : register(b2)
{
	float4x4 array[36];
}

V_OUT main(V_IN input, uint id:SV_InstanceID)
{
	V_OUT output = (V_OUT)0;

	float4 localH = float4(input.posL.xyz, 1);

	output.worldPos = localH.xyzw;

	localH = mul(localH, array[id]);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);

	output.posH = localH;
	output.uv = input.uv;
	output.normal = mul(float4(input.normal.xyz, 0), array[id]);

	return output;
}