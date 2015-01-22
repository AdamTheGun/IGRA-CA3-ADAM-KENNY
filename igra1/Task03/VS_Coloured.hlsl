// this is what goes into the vertex shader
struct VS_INPUT
{
	float4 pos : POSITION; // the vertex buffer (C++ is only float3) but we will use a float4 in the shaders because it's easier
						   // the input layout will perform the changes for us
	float4 col : COLOR;
};

// what comes out of the vertex shader
struct VS_OUTPUT
{
	float4 pos : SV_POSITION; // position of screen coordinates
	float4 col : COLOR; // colour again
};

// out shader now takes an in struct & an out struct
VS_OUTPUT main (VS_INPUT vIn)
{
	VS_OUTPUT vOut;

	vOut.pos = vIn.pos;
	vOut.col = vIn.col;

	return vOut;
}