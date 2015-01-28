/******************************
* Task6a.cpp
* Written by The IGRA team
******************************/
#include "DxCommon.h"	// common DX stuff
#include <DirectXColors.h>	// DirectX::Colors

#include "IgraApp.h"	// the IGRA 'game' class
#include "IgraUtils.h"	// useful functions
#include "ShaderManager.h"	// holds all the shaders
#include "ToString.h"
#include "SimpleMathToString.h"
#include "SpriteBatch.h" 
#include "SpriteFont.h"
#include "Draw3D.h"

#include "Vertex.h"	// the IGRA vertex structures

#include "GeometryGenerator.h"	// the IGRA vertex structures
# define M_PI           3.14159265358979323846  /* pi */
#define D3DXToRadian(degree) ((degree) * (M_PI / 180.0f))
#include "ModelObj.h"	// the IGRA model loader

using namespace DirectX::Colors;
using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace igra;

class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	//void Shutdown();	

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;
	std::unique_ptr<PrimitiveBatch<ColouredVertex>> mpDraw3D;

	CComPtr<ID3D11ShaderResourceView> mpCubemap;
	ModelObj mTeapot,mPlane,mSkyObj;
	
	void DrawShadows(const Matrix& view,const Matrix& proj) ;
	BasicMaterial mShadowMaterial;
	CComPtr<ID3D11DepthStencilState> mpShadowStencil;

	ArcBallCamera mCamera;
};

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 6a: Model Loading";

	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	
	// initial pos/tgt for camera
	mCamera.Reset();

	// load models:
	mTeapot.Load(GetDevice(),mpShaderManager.get(),L"../Content/dragon.obj",
                         true);
	mPlane.Load(GetDevice(),mpShaderManager.get(),L"../Content/plane.obj",true);
	mSkyObj.Load(GetDevice(),mpShaderManager.get(),
                               L"../Content/skyball.obj",false);
	mpCubemap.Attach(CreateTextureResourceDDS(GetDevice(),
                               L"../Content/Cubemaps/grasscube1024.dds"));

	mShadowMaterial.mpShaderManager=mpShaderManager.get();
	mShadowMaterial.mpVS=mpShaderManager->VSDefault();
	mShadowMaterial.mpLayout=mpShaderManager->LayoutDefault();
	mShadowMaterial.mpPS=mpShaderManager->PSUnlit();
	mShadowMaterial.mMaterial.gEnableReflection=false;
	mShadowMaterial.mMaterial.gUseTexture=false;
	mShadowMaterial.mMaterial.gMaterial=MakeMaterial(Color(0,0,0,0.5f),
                Colors::Black); // trans black, no specular

	// create the shadow stencil
	D3D11_DEPTH_STENCIL_DESC dsd;
	ZeroMemory(&dsd, sizeof(dsd));
	// enable depth testing
	dsd.DepthEnable=true; 
	dsd.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc=D3D11_COMPARISON_LESS;
	// turn on stencil buffer:
	dsd.StencilEnable=true;
	dsd.StencilReadMask=dsd.StencilWriteMask=0xff;
	// if the value is equal to Value then inc, otherwise don't change it
	dsd.FrontFace.StencilFunc=			D3D11_COMPARISON_EQUAL;
	dsd.FrontFace.StencilPassOp=		D3D11_STENCIL_OP_INCR;
	dsd.FrontFace.StencilFailOp=		D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp=		D3D11_STENCIL_OP_KEEP;
	// we will not render back face, so just put some safe values in
	dsd.BackFace=dsd.FrontFace;
	// make it:
	GetDevice()->CreateDepthStencilState(&dsd,&mpShadowStencil);


	mSkyObj.mMaterial.mpVS=mpShaderManager->VSSkybox();
	mSkyObj.mMaterial.mpLayout=mpShaderManager->LayoutSkybox();
	mSkyObj.mMaterial.mpPS=mpShaderManager->PSSkybox();
	mSkyObj.mMaterial.mpTexture=mpCubemap;
	mSkyObj.mMaterial.mMaterial.gUseTexture=true;

	mTeapot.mMaterial.mMaterial.gMaterial.Specular=Color(1,1,1,1);
	mTeapot.mMaterial.mMaterial.gMaterial.Specular.w=20;	// power
	mTeapot.mMaterial.mMaterial.gMaterial.Reflect=Color(0.1,0.1,1,1)*0.5f;
	mTeapot.mMaterial.mMaterial.gEnableReflection=true;

}
void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),
                                  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(),
                                  Colors::SkyBlue);
	// Set common rendering flags
	GetContext()->IASetPrimitiveTopology(
                            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	GetContext()->OMSetDepthStencilState(
                            mpShaderManager->CommonStates()->DepthDefault(),0);
	ShaderManager::SetSampler(GetContext(),
                            mpShaderManager->CommonStates()->LinearWrap());

	GetContext()->OMSetDepthStencilState(mpShadowStencil,0);

	// setup the matrixes
	Matrix world;
	Matrix view=mCamera.GetViewMatrix();
	Matrix proj=mCamera.GetDefaultProjectionMatrix();
	// the models:
	world=Matrix::CreateTranslation(0,0,0);
	mPlane.mMaterial.FillMatrixes(world,view,proj);
	mPlane.mMaterial.Apply(GetContext());
	mPlane.Draw(GetContext());

	GetContext()->OMSetDepthStencilState(nullptr,0);

	world=Matrix::CreateTranslation(0,0,0);
	mTeapot.mMaterial.FillMatrixes(world,view,proj);
	mTeapot.mMaterial.mLights.gEyePosW=mCamera.GetCamPos();
	mTeapot.mMaterial.Apply(GetContext());
	// EXTRA: add the cubemap as T1
	ID3D11ShaderResourceView* tex[]={mpCubemap};
	GetContext()->PSSetShaderResources(1,1,tex);	// set as index T1
	mTeapot.Draw(GetContext());
	DrawShadows(view,proj);
	// draw sky
	world= Matrix::CreateTranslation(mCamera.GetCamPos());
	mSkyObj.mMaterial.FillMatrixes(world,view,proj);
	mSkyObj.mMaterial.Apply(GetContext());
	mSkyObj.Draw(GetContext());
	// Present the backbuffer to the screen

	
	GetSwapChain()->Present(0, 0);

}
void MyApp::DrawShadows(const Matrix& view,const Matrix& proj)
{
	Matrix world;
	// create shadow matrix:
	Plane ground(Vector3(0,1,0),0); // ground's normal is up & pos is 0
	// getting the strongest light source (gDirLights[0]) to make the shadow
	Vector3 toLight=-mTeapot.mMaterial.mLights.gDirLights[0].Direction;
	// make matrix (see Luna for the math details)
	Matrix shadowMatrix=Matrix::CreateShadow(toLight,ground);
	// add a very small offset to keep it out of the ground
	shadowMatrix*=Matrix::CreateTranslation(0,0.01f,0);

	const float BLEND_FACTOR[] = {0.0f, 0.0f, 0.0f, 0.0f};
	const unsigned BLEND_MASK=0xffffffff;
	GetContext()->OMSetBlendState(
            mpShaderManager->CommonStates()->NonPremultiplied(),
            BLEND_FACTOR,BLEND_MASK);
	GetContext()->OMSetDepthStencilState(mpShadowStencil,0);

	// draw objects almost as normal:
	// just using a different material & a different world matrix

	GetContext()->OMSetDepthStencilState(mpShadowStencil,1);

	world=Matrix::CreateTranslation(0,1,0) * shadowMatrix;
	mShadowMaterial.FillMatrixes(world,view,proj);
	mShadowMaterial.Apply(GetContext());
	mTeapot.Draw(GetContext());
	// draw all the rest using the world*shadowMatrix

	// turn off blend 
	GetContext()->OMSetBlendState(
               mpShaderManager->CommonStates()->Opaque(),
               BLEND_FACTOR,BLEND_MASK);
	GetContext()->OMSetDepthStencilState(nullptr,0);
}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	mCamera.Update();
}


// in console C++ is was main()
// in Windows C++ its called WinMain()  (or sometimes wWinMain)
int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine,
	int nShowCmd)
{
	MyApp app;
	return app.Go(hInstance);	// go!
}



