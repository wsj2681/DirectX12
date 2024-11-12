struct PixelInput
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

// 픽셀 셰이더
float4 PSMain(PixelInput input) : SV_TARGET
{
	return float4(input.color, 1.0f); // 각 정점의 색상 사용
}