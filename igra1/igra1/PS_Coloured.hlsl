


// what comes out of the vertex shader
struct VS_OUTPUT
{
	float4 pos : SV_POSITION; // position of screen coordinates
	float4 col : COLOR; // colour again
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return input.col;
}
