/*************************
* Skybox Shader info:
* Written by <Your name>
*************************/

// input to the VS: we only care about the position
// not even the texture coordinates
struct SkyboxIn
{
	float3 PosL : POSITION;
};

// output of VS, input to PS
// it looks quite different from usual
struct SkyboxOut
{
	float4 PosH : SV_POSITION; // position transformed by all the matrixes
	float3 PosL : POSITION; // original model position
};

// Constant buffer
// identical to the one used in VS_Default.hlsl for compatibility
cbuffer cbMatrixInfo
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
}; 

// vertex shader
SkyboxOut VS( SkyboxIn vin )
{
	SkyboxOut vout;
	
	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj).xyww;
	
	// Use local vertex position as cubemap lookup vector.
	vout.PosL = vin.PosL;

	return vout;
}

// texture and sampler as usual
TextureCube gCubeMap;
SamplerState gSampler;

float4 PS(SkyboxOut pin) : SV_TARGET
{
	// sample using the vertex position to get the correct texture coordinate	
	return gCubeMap.Sample(gSampler, pin.PosL);
}
