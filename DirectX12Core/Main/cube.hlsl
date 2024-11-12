// cube.hlsl
cbuffer MVP : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VSInput
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
	PSInput output;
	float4 pos = float4(input.position, 1.0);
	output.position = mul(pos, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);
	output.color = float4(1.0, 1.0, 0.0, 1.0);
	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.color;
}