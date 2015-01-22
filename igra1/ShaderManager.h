/*==============================================
 * IGRA Shader Manager
 *
 * Written by <YOUR NAME HERE>
 *==============================================*/
#pragma once

#include "DxCommon.h"	// Common Dx stuff
#include "LightHelper.h"		// lighting structures

namespace igra
{

/** The ShaderManager is a single class to hold & manage ALL the shaders in an IGRA application.
It also holds all the helper fns to load & manage the shaders & the shaders constant buffers.

As the semester progresses this class will expand, adding more shaders in.
YOU MUST NOT remove/change the existing shaders as this will break back compatibity with the earlier examples.

When adding shaders into the build:
* Copy shader into Igra/Shaders
* Add then into the project file (RMB->Add Existing)
* Set properties: (RMB->Properties)
** Change Object file name from:	$(OutDir)%(Filename).cso to ..\Content\%(Filename).cso
** That way all shaders appear in the Content directory
** Set the Shader Type accordingly (VS or PS)
** If needed set the Shader Model

*/
class ShaderManager
{
public:
	ShaderManager(ID3D11Device* pDevice);
	~ShaderManager();

public:	// general accessors

	// CommonStates
	DirectX::CommonStates* CommonStates(){return mpCommonStates.get();}

	// coloured VS/PS/IA for Draw3D
	ID3D11VertexShader* VSColoured(){return mpVSColoured;}
	ID3D11InputLayout*	LayoutColoured(){return mpLayoutColoured;}
	ID3D11PixelShader*	PSColoured(){return mpPSColoured;}

	// default shaders/layouts:
	ID3D11VertexShader*	VSDefault(){return mpVSDefault;}
	ID3D11InputLayout*	LayoutDefault(){return mpLayoutDefault;}
	ID3D11PixelShader*	PSUnlit(){return mpPSUnlit;}
	ID3D11PixelShader*	PSDummyLight(){return mpPSDummyLight;}
	ID3D11PixelShader*	PSDirLight(){return mpPSDirLight;}

	// default const buffers
	ID3D11Buffer*		CBMatrixBasic(){return mpCBMatrixBasic;}	
	ID3D11Buffer*		CBMatrixInfo(){return mpCBMatrixInfo;}
	ID3D11Buffer*		CBMaterial(){return mpCBMaterial;}
	ID3D11Buffer*		CBLights(){return mpCBLights;}

public:		// various constant buffers used in the shaders

	// simplified matrix info for VS_Coloured
	struct cbMatrixBasic
	{
		DirectX::XMMATRIX gWorldViewProj;
	}; 

	// Matrix info used in most vertex shaders
	struct cbMatrixInfo
	{
		DirectX::XMMATRIX gWorld;
		DirectX::XMMATRIX gWorldInvTranspose;
		DirectX::XMMATRIX gWorldViewProj;
	}; 

	// Material info used in most pixel shaders
	struct cbMaterial
	{
		igra::Material gMaterial;
		BOOL gUseTexture;// needs to be a 4 bytes, so using BOOL rather than bool
		BOOL pad1,pad2,pad3;	// need 12 more bytes (3 BOOL's) as padding
	}; 

	struct cbLights
	{
		igra::DirectionalLight gDirLights[3];
		DirectX::XMFLOAT3 gEyePosW;
		float pad0;	// needs one extra float
	};

public:	// static helpers:

	/// loads a .cso file into memory
	static ID3DBlob* LoadShaderCso(const wchar_t* filename);
	/// create a VS from a CSO blob
	static ID3D11VertexShader* CreateVertexShaderFromBlob(ID3D11Device* pDevice,ID3DBlob* pBlob);
	/// create a Input Layout from a CSO blob
	static ID3D11InputLayout* CreateLayoutFromBlob(ID3D11Device* pDevice,ID3DBlob* pBlob,const D3D11_INPUT_ELEMENT_DESC layout[],unsigned layoutLen);
	/// creates a PS from a CSO file
	static ID3D11PixelShader* CreatePixelShaderFromCso(ID3D11Device* pDevice,const wchar_t* filename);
	/// creates a constant buffer for a shader
	static ID3D11Buffer* CreateConstantBuffer(ID3D11Device* pDev,unsigned buffer_size);
	/// updates a constant buffer for the vertex shader
	static void SetVSConstantBuffer(ID3D11DeviceContext* pContext,ID3D11Buffer* pBuffer,void* pData,unsigned index=0);
	/// updates a constant buffer for the pixel shader
	static void SetPSConstantBuffer(ID3D11DeviceContext* pContext,ID3D11Buffer* pBuffer,void* pData,unsigned index=0);
	/// sets vertex & index buffers
	static void SetVertexIndexBuffers(ID3D11DeviceContext* pContext,ID3D11Buffer* pVB,unsigned sizeofVertex,ID3D11Buffer* pIB);
	/// sets shader resource view (texture)
	static void SetShaderResource(ID3D11DeviceContext* pContext,ID3D11ShaderResourceView* pSrv,unsigned index=0);
	/// sets shader sampler
	static void SetSampler(ID3D11DeviceContext* pContext,ID3D11SamplerState* pSamp);
	/// sets shader resource view (texture) & sampler
	static void SetShaderResourceSampler(ID3D11DeviceContext* pContext,ID3D11ShaderResourceView* pSrv,ID3D11SamplerState* pSamp);
	

private:	// internal data

	std::unique_ptr<DirectX::CommonStates> mpCommonStates;

	// ColouredVertex shader/layouts
	CComPtr<ID3D11VertexShader> mpVSColoured;
	CComPtr<ID3D11InputLayout> mpLayoutColoured;
	CComPtr<ID3D11PixelShader> mpPSColoured;	

	// default shaders/layouts/buffers
	CComPtr<ID3D11VertexShader> mpVSDefault;
	CComPtr<ID3D11InputLayout> mpLayoutDefault;
	CComPtr<ID3D11PixelShader> mpPSUnlit;
	CComPtr<ID3D11PixelShader> mpPSDummyLight;
	CComPtr<ID3D11PixelShader> mpPSDirLight;

	// const buffers
	CComPtr<ID3D11Buffer> mpCBMatrixInfo,mpCBMatrixBasic,mpCBMaterial;
	CComPtr<ID3D11Buffer> mpCBLights;

private:
	ShaderManager(const ShaderManager&);	/// NO COPYING
	void operator=(const ShaderManager&);	/// NO NO COPYING
};	// class


/** Base class for shader management.
Holds vertex & pixel shader and input layout.
Allows setting the the 3 matrixes (WVP).
Derived classes can add in their additional features
*/
class ShaderMaterial
{
public:	// following variables MUST be set before use
	ShaderManager* mpShaderManager;
	ID3D11InputLayout* mpLayout;// so many layouts, you need to select:
	ID3D11VertexShader* mpVS;	// so many vertex shaders, you need to select:
	ID3D11PixelShader* mpPS;	// so many pixel shaders, you need to select:

public:	// common functions
	/// fills all the matrix information
	void FillMatrixes(const DirectX::SimpleMath::Matrix& world,
                         const DirectX::SimpleMath::Matrix& view,
                         const DirectX::SimpleMath::Matrix& projection);
	// sets the shaders & layout in the context
	void SetShaderLayout(ID3D11DeviceContext* pContext);

	// sets the matrix information in the shader's constant buffer
	void SetVsCBuffer(ID3D11DeviceContext* pContext);
protected:
	ShaderManager::cbMatrixInfo mMatrixInfo;	/// the matrix buffer data
};
/** Basic material.
Works with lights, coloured objects & optional textures.
Does not support normal maps or other advanced features.

If you are not sure which to use, this is probably it.
*/
class BasicMaterial: public ShaderMaterial
{
public:	// following variables MUST be set before use
	BasicMaterial(){mMaterial.gUseTexture = false;}
	ShaderManager::cbMaterial mMaterial;		/// the material
	ShaderManager::cbLights mLights;
public:	// common functions
	// helper to setup default lighting variables
	void SetupDefaultLights();
	// sets the matrix information in the pixel shader's constant buffer
	void SetPsCBuffer(ID3D11DeviceContext* pContext);
	// applies layout,shaders,VS-Cbuff,PS-CBuff & Textures(no samplers)
	void Apply(ID3D11DeviceContext* pContext);
};



}	// namespace