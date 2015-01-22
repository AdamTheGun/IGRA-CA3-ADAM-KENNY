/*************************
* VS_default.hlsl
* Default Vertex Shader:
* Written by IGRA team.
* Suitable for lights, textures, normal maps & more
*************************/

// light info & vertex format
#include "LightHelper.hlsli"

// keep format for compatibility with other shaders
// matches ShaderManager::cbMatrixInfo
cbuffer cbMatrixInfo
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
}; 

VertexOut main( VertexIn vin)
{
	VertexOut vout;
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);

	// process tangent, just in case this is a normal mapped item
	vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);

	// texture coords are as-is
	vout.Tex=vin.Tex;

	return vout;
}
