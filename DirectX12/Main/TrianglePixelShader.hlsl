struct PixelInput
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

// �ȼ� ���̴�
float4 PSMain(PixelInput input) : SV_TARGET
{
	return float4(input.color, 1.0f); // �� ������ ���� ���
}