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

// Á¤Á¡ ¼ÎÀÌ´õ
PixelInput VSMain(VertexInput input)
{
	PixelInput output;
	output.position = float4(input.position, 1.0f);
	output.color = input.color;
	return output;
}

// ÇÈ¼¿ ¼ÎÀÌ´õ
float4 PSMain(PixelInput input) : SV_TARGET
{
	return float4(input.color, 1.0f); // °¢ Á¤Á¡ÀÇ »ö»ó »ç¿ë
}