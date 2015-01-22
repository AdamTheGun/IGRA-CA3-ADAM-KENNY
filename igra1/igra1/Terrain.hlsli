/*************************
* Terrain Shader info: Terrain.hlsli
* Written by <You name>
*************************/

// Shader input/output:

// input to VS
// MUST MATCH the C++ struct TerrainVertex
struct TerrainVertexIn
{
	float3 PosL    : POSITION;
	float2 Tex     : TEXCOORD;
};

// output of VS, input to PS
// its actually quite a simple struct
struct TerrainVertexOut
{
	float4 PosH    : SV_POSITION;
	float2 Tex     : TEXCOORD;
};

// Vertex Shader constant buffer:
// matches ShaderManager::cbMatrixInfo & cbMatrixInfo in VS_default.hlsl
// to simplify matters
cbuffer cbMatrixInfo
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
}; 

SamplerState ObjSamplerState;		// used in all PS's
Texture2D gBlendMap: register(t0);	// used in all PS's
Texture2DArray gLayerMapArray: register(t1);	// only used in PS_MULTI_TEX
Texture2D gLightMap: register(t2);	

float4 PS_MULTI_TEX(TerrainVertexOut pin) : SV_TARGET
{
	float4 color;
	// sample from each layer in the texture array
	// note: TEX_SCALE is used to repeat the texture
	const float TEX_SCALE=16;
	float4 c0=gLayerMapArray.Sample(ObjSamplerState, 
                               float3(pin.Tex*TEX_SCALE,0));
	float4 c1=gLayerMapArray.Sample(ObjSamplerState, 
                               float3(pin.Tex*TEX_SCALE,1));
	float4 c2=gLayerMapArray.Sample(ObjSamplerState, 
                               float3(pin.Tex*TEX_SCALE,2));
	float4 c3=gLayerMapArray.Sample(ObjSamplerState, 
                               float3(pin.Tex*TEX_SCALE,3));
	// get the blend info
	float4 b=gBlendMap.Sample(ObjSamplerState, pin.Tex);

	// blend the colours using a simple multiply
	// sometimes lerp() is good, but multiply works fine here
	color = (c0 * b.r) + (c1 * b.g) + (c2 * b.b) + (c3 * b.a);
	// add light
	// the light texture is only 8 bit greyscale, so just use the .r bit
	color*=gLightMap.Sample(ObjSamplerState, pin.Tex).r;

	return float4(color.r,color.g,color.b,color.a);

}

// basic pixel shader
float4 PS_BASIC(TerrainVertexOut pin) : SV_TARGET
{
	float4 color = gBlendMap.Sample( ObjSamplerState, pin.Tex );
	return color;
}

// vertex shader code
TerrainVertexOut VS( TerrainVertexIn vin )
{
	TerrainVertexOut vout;
	// Transform to homogeneous clip space (the screen space)
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// texture, as is
	vout.Tex=vin.Tex;
	
	return vout;
}
