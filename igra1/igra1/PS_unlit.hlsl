/*************************
* PS_unlit.hlsl
* Unlit Pixel Shader:
* Written by IGRA team.
*************************/

#include "LightHelper.hlsli"
 
// matches ShaderManager::cbMaterial
cbuffer cbMaterial : register(b0)
{
	Material gMaterial;		// the material
	bool gUseTexture;	// whether we use a texture or just colour
	bool pad1,pad2,pad3;	// padding (ignore)
}; 
// Texture2D & SamplerState cannot be in a buffer
// they must be accessed directly
Texture2D gDiffuseTexture: register(t0);
SamplerState gSamplerState;



float4 main(VertexOut pin) : SV_Target
{
	// just diffuse colour
	float4 litColor = gMaterial.Diffuse;
		
	if (gUseTexture)
		litColor *= gDiffuseTexture.Sample( gSamplerState, pin.Tex );

	return litColor;
}
