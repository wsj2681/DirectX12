struct VertexInput
{
	float3 position : POSITION;
	float3 color : COLOR;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

// ���� ���̴�
PixelInput VSMain(VertexInput input)
{
	PixelInput output;
	output.position = float4(input.position, 1.0f);
	output.color = input.color;
	return output;
}
