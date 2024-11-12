// Shaders.hlsl
float4 VSMain(uint vertexID : SV_VertexID) : SV_POSITION
{
	float4 vertices[3] =
	{
		float4(0.0f, 0.5f, 0.5f, 1.0f), // Top vertex
        float4(0.5f, -0.5f, 0.5f, 1.0f), // Bottom right
        float4(-0.5f, -0.5f, 0.5f, 1.0f) // Bottom left
	};
	return vertices[vertexID];
}

float4 PSMain(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow color
}