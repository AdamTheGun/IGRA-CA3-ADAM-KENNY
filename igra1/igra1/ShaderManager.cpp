/*==============================================
 * IGRA Shader Manager
 *
 * Written by <YOUR NAME HERE>
 *==============================================*/

// header
#include "ShaderManager.h"
#include "IgraApp.h"	// error handling
#include "Vertex.h"	// Vertex descriptions

#include <d3dcompiler.h>	// shader loading
#include "SimpleMath.h"

// link appropriate libraries 
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX::SimpleMath;

namespace igra
{

ShaderManager::ShaderManager(ID3D11Device* pDevice)
{
	// add common states
	mpCommonStates.reset(new DirectX::CommonStates(pDevice));

	// load a LOT of shaders:
	CComPtr<ID3DBlob> pBlob;	// temp blob

	// ColouredVertex shader/layouts
	pBlob=LoadShaderCso(L"../Content/Shaders/VS_Coloured3D.cso");
	mpVSColoured=CreateVertexShaderFromBlob(pDevice,pBlob);
	mpLayoutColoured=CreateLayoutFromBlob(pDevice,pBlob,ColouredVertex_layout,ColouredVertex_layout_len);
	mpPSColoured=CreatePixelShaderFromCso(pDevice,L"../Content/Shaders/PS_Coloured.cso");

	// default shaders/layouts/buffers
	pBlob=LoadShaderCso(L"../Content/Shaders/VS_Default.cso");
	mpVSDefault=CreateVertexShaderFromBlob(pDevice,pBlob);
	mpLayoutDefault=CreateLayoutFromBlob(pDevice,pBlob,Vertex_layout,
                            Vertex_layout_len);
	mpPSUnlit=CreatePixelShaderFromCso(pDevice,
                            L"../Content/Shaders/PS_Unlit.cso");
	mpPSDummyLight=CreatePixelShaderFromCso(pDevice,
                          L"../Content/Shaders/PS_DummyLight.cso");
	mpPSDirLight=CreatePixelShaderFromCso(pDevice,
                                   L"../Content/Shaders/PS_DirLight.cso");
	
	pBlob=LoadShaderCso(L"../Content/Shaders/VS_Terrain.cso");
	mpVSTerrain=CreateVertexShaderFromBlob(pDevice,pBlob);
	mpLayoutTerrain=CreateLayoutFromBlob(pDevice,pBlob,
                         TerrainVertex_layout,TerrainVertex_layout_len);
	mpPSTerrain=CreatePixelShaderFromCso(pDevice,
                         L"../Content/Shaders/PS_TerrainMulti.cso");
	mpPSTerrainBasic=CreatePixelShaderFromCso(pDevice,
                         L"../Content/Shaders/PS_TerrainBasic.cso");
	// skybox
	pBlob.Attach(LoadShaderCso(L"../Content/Shaders/VS_Skybox.cso"));
	mpVSSkybox=CreateVertexShaderFromBlob(pDevice,pBlob);
	mpLayoutSkybox=CreateLayoutFromBlob(pDevice,pBlob,
                                  Vertex_layout,Vertex_layout_len);
	mpPSSkybox=CreatePixelShaderFromCso(pDevice,
                                  L"../Content/Shaders/PS_Skybox.cso");


	// const buffers
	mpCBMatrixBasic=CreateConstantBuffer(pDevice,sizeof(cbMatrixBasic));
	mpCBMatrixInfo=CreateConstantBuffer(pDevice,sizeof(cbMatrixInfo));
	mpCBMaterial=CreateConstantBuffer(pDevice,sizeof(cbMaterial));
	mpCBLights=CreateConstantBuffer(pDevice,sizeof(cbLights));
}

ShaderManager::~ShaderManager()
{
}

void TerrainMaterial::Apply(ID3D11DeviceContext* pContext)
{
	// select shaders
	pContext->IASetInputLayout(mpShaderManager->LayoutTerrain());	
	pContext->VSSetShader(mpShaderManager->VSTerrain(), 0, 0);	
	
	if (mUseTextureBlend)
		pContext->PSSetShader(mpShaderManager->PSTerrain(), 0, 0);		
	else
		pContext->PSSetShader(mpShaderManager->PSTerrainBasic(), 0, 0); 

	// fill & set constant buffer
	SetVsCBuffer(pContext);	// the VS buffers
	// there is no CB for pixel shaders

	// set textures & sampler
	ID3D11ShaderResourceView* texs[]={mpTexBase,mpTexLayerArray,mpTexLight};
	pContext->PSSetShaderResources(0,3,texs); 

	ID3D11SamplerState* samps[]={
                              mpShaderManager->CommonStates()->LinearWrap()};
	pContext->PSSetSamplers(0,1,samps);

}


ID3DBlob* ShaderManager::LoadShaderCso(const wchar_t* filename)
{
	ID3DBlob* pBlob;
	HR_MSG(D3DReadFileToBlob(filename,&pBlob),filename);
	return pBlob;
}

ID3D11VertexShader* ShaderManager::CreateVertexShaderFromBlob(ID3D11Device* pDevice,ID3DBlob* pBlob)
{
	ID3D11VertexShader* pVS=nullptr;
	HR(pDevice->CreateVertexShader(pBlob->GetBufferPointer(),pBlob->GetBufferSize(),nullptr,&pVS));
	return pVS;
}

ID3D11InputLayout* ShaderManager::CreateLayoutFromBlob(ID3D11Device* pDevice,ID3DBlob* pBlob,const D3D11_INPUT_ELEMENT_DESC layout[],unsigned layoutLen)
{
	ID3D11InputLayout* pLayout=nullptr;
	HR(pDevice->CreateInputLayout(layout,layoutLen,pBlob->GetBufferPointer(),pBlob->GetBufferSize(),&pLayout));
	return pLayout;
}

ID3D11PixelShader* ShaderManager::CreatePixelShaderFromCso(ID3D11Device* pDevice,const wchar_t* filename)
{
	ID3D11PixelShader* pPS=nullptr;
	CComPtr<ID3DBlob> pBlob;
	HR(D3DReadFileToBlob(filename,&pBlob));
	HR(pDevice->CreatePixelShader(pBlob->GetBufferPointer(),pBlob->GetBufferSize(),nullptr,&pPS));
	return pPS;
}

ID3D11Buffer* ShaderManager::CreateConstantBuffer(ID3D11Device* pDev,unsigned buffer_size)
{
	ID3D11Buffer* pBuffer=nullptr;
	D3D11_BUFFER_DESC cbbd;	
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = buffer_size;
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR(pDev->CreateBuffer(&cbbd, NULL, &pBuffer));
	return pBuffer;
}

void ShaderManager::SetVSConstantBuffer(ID3D11DeviceContext* pContext,ID3D11Buffer* pBuffer,void* pData,unsigned index)
{
	pContext->UpdateSubresource( pBuffer, 0, NULL, pData, 0, 0 );
	ID3D11Buffer* buffers[]={pBuffer};
	pContext->VSSetConstantBuffers(index, 1, buffers);	// note: update the register(index)
}

void ShaderManager::SetPSConstantBuffer(ID3D11DeviceContext* pContext,ID3D11Buffer* pBuffer,void* pData,unsigned index)
{
	pContext->UpdateSubresource( pBuffer, 0, NULL, pData, 0, 0 );
	ID3D11Buffer* buffers[]={pBuffer};
	pContext->PSSetConstantBuffers(index, 1, buffers);	// note: update the register(index)
}

void ShaderManager::SetVertexIndexBuffers(ID3D11DeviceContext* pContext,ID3D11Buffer* pVB,unsigned sizeofVertex,ID3D11Buffer* pIB)
{
	UINT stride = sizeofVertex; //size of 1x Vertex in bytes
	UINT offset = 0;
	ID3D11Buffer* buffers[]={pVB}; //array of VB pointers
	pContext->IASetVertexBuffers(0,1,buffers, &stride, &offset);

	pContext->IASetIndexBuffer(pIB,DXGI_FORMAT_R32_UINT,0);
}

void ShaderManager::SetShaderResource(ID3D11DeviceContext* pContext,ID3D11ShaderResourceView* pSrv,unsigned index)
{
	ID3D11ShaderResourceView* srvs[]={pSrv};
	pContext->PSSetShaderResources(index,1,srvs);
}

void ShaderManager::SetSampler(ID3D11DeviceContext* pContext,ID3D11SamplerState* pSamp)
{
	ID3D11SamplerState* samps[]={pSamp};
	pContext->PSSetSamplers(0,1,samps);
}

void ShaderManager::SetShaderResourceSampler(ID3D11DeviceContext* pContext,ID3D11ShaderResourceView* pSrv,ID3D11SamplerState* pSamp)
{
	SetShaderResource(pContext,pSrv);
	SetSampler(pContext,pSamp);
}


////////////////////////////////////////////////////////////////////////////////
void ShaderMaterial::FillMatrixes(const DirectX::SimpleMath::Matrix& world,
                                  const DirectX::SimpleMath::Matrix& view,
                                  const DirectX::SimpleMath::Matrix& projection)
{
	mMatrixInfo.gWorld=world.Transpose();
	mMatrixInfo.gWorldInvTranspose=world.Invert();
	// because all Matrix had to be transposed, 
       //this invert-transpose-transpose=just invert
	mMatrixInfo.gWorldViewProj=(world*view*projection).Transpose();
}

void ShaderMaterial::SetShaderLayout(ID3D11DeviceContext* pContext)
{
	// select shaders
	pContext->IASetInputLayout(mpLayout);	
	pContext->VSSetShader(mpVS, 0, 0);	
	pContext->PSSetShader(mpPS, 0, 0);	
}

void ShaderMaterial::SetVsCBuffer(ID3D11DeviceContext* pContext)
{
	ShaderManager::SetVSConstantBuffer(pContext,
          mpShaderManager->CBMatrixInfo(),&mMatrixInfo);
}
////////////////////////////////////////////////////////////////////////////////
void BasicMaterial::SetupDefaultLights()
{

	ZeroMemory(&mLights,sizeof(mLights));
	mLights.gDirLights[0].Ambient  = Color(0.3f, 0.3f, 0.3f, 1.0f);
	mLights.gDirLights[0].Diffuse  = Color(0.9f, 0.9f, 0.9f, 1.0f);
	mLights.gDirLights[0].Specular = Color(0.3f, 0.3f, 0.3f, 1.0f);
	mLights.gDirLights[0].Direction = Vector3(0.57735f, -0.97735f, 0.57735f);

	mLights.gDirLights[1].Ambient  = Color(0.3f, 0.3f, 0.3f, 1.0f);
	mLights.gDirLights[1].Diffuse  = Color(0.90f, 0.90f, 0.90f, 1.0f);
	mLights.gDirLights[1].Specular = Color(0.15f, 0.15f, 0.15f, 1.0f);
	mLights.gDirLights[1].Direction = Vector3(-0.57735f, -0.57735f,0.57735f);

	mLights.gDirLights[2].Ambient  = Color(0.3f, 0.3f, 0.3f, 1.0f);
	mLights.gDirLights[2].Diffuse  = Color(0.9f, 0.9f, 0.9f, 1.0f);
	mLights.gDirLights[2].Specular = Color(0.0f, 0.0f, 0.0f, 1.0f);
	mLights.gDirLights[2].Direction = Vector3(0.0f, -0.707f, -0.707f);

	mLights.gDirLights[0].Direction.Normalize();
	mLights.gDirLights[1].Direction.Normalize();
	mLights.gDirLights[2].Direction.Normalize();
}

void BasicMaterial::SetPsCBuffer(ID3D11DeviceContext* pContext)
{
	// for pixel shader its 2 buffers:
	pContext->UpdateSubresource( mpShaderManager->CBMaterial(), 0, NULL,
                                     &mMaterial, 0, 0 );	// update
	pContext->UpdateSubresource( mpShaderManager->CBLights(), 0, NULL,
                                     &mLights, 0, 0 );	// update
	ID3D11Buffer* buffers[]={mpShaderManager->CBMaterial(),
                   mpShaderManager->CBLights()};	// material is b0, lights is b1
	pContext->PSSetConstantBuffers(0,2, buffers);	// update both
}

void BasicMaterial::SetTexture(ID3D11DeviceContext* pContext)
{
	if(mMaterial.gUseTexture)
	{
		ShaderManager::SetShaderResource(pContext,mpTexture);
	}
}

void BasicMaterial::Apply(ID3D11DeviceContext* pContext)
{
	SetShaderLayout(pContext);
	SetVsCBuffer(pContext);
	SetPsCBuffer(pContext);
	SetTexture(pContext);
}

} // namespace igra

