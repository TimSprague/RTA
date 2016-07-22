struct P_IN
{
	float4 posH : SV_POSITION;
	float4 col : COLOR;
};

float4 main(P_IN input1) : SV_TARGET
{
	float4 color = input1.col;

	return color;
}