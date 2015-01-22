/******************************
* PS_DummyLight.hlsl
* Written by <Your name here>
******************************/

#include "LightHelper.hlsli"
 
// matches ShaderManager::cbMaterial
cbuffer cbMaterial : register(b0)
{
	Material gMaterial;		// the material
	bool gUseTexture;// needs to be a 4 bytes, so using BOOL rather than bool
	bool gReflectionEnabled;
	bool pad1,pad2,pad3;	// need 12 more bytes (3 BOOL's) as padding
}; 

Texture2D gDiffuseTexture: register(t0);
SamplerState gSamplerState;

float4 main(VertexOut pin) : SV_Target
{
	// the normal 
	float3 normal=normalize(pin.NormalW);
	// light direction (hard coded)
	float3 lightDir=normalize(float3(1,0.3,0));
	// compute the light amount
	float light=dot(normal,lightDir);
	light=max(light,0);

	// just diffuse colour
	float4 litColor = gMaterial.Diffuse;
	// include light
	litColor *= light;

		if (gUseTexture)
		litColor *= gDiffuseTexture.Sample( gSamplerState, pin.Tex );

    return litColor;
}
