/******************************
* Task4a.cpp
* Written by The IGRA team
* Uses the core IGRA code (ShaderManager & others)
* to draw the 2 moving cubes
******************************/

#include "DxCommon.h"	// common DX stuff
#include <DirectXColors.h>	// DirectX::Colors

#include "IgraApp.h"	// the IGRA 'game' class
#include "IgraUtils.h"	// useful functions
#include "ShaderManager.h"	// holds all the shaders
#include "GeometryGenerator.h"

#include "Vertex.h"	// the IGRA vertex structures

using namespace igra;
using namespace DirectX;	// XMVECTOR
using namespace DirectX::SimpleMath;	// for Color

class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	//void Shutdown();	

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;

	CComPtr<ID3D11Buffer> mpVertexBuffer;	// vertex buffer
	CComPtr<ID3D11Buffer> mpIndexBuffer;	// index buffer
	unsigned mNumIndex;						// number of indexes

	ArcBallCamera mCamera;
};

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 4a: The Cubes, with Shader Manager";

	//Create the vertex buffer
	ColouredVertex v[] =
	{
		{Vector3( -1, -1, -1 ),Colors::Black},
		{Vector3( +1, -1, -1 ),Colors::Red},
		{Vector3( -1, +1, -1 ),Colors::Green},
		{Vector3( +1, +1, -1 ),Colors::Yellow},
		{Vector3( -1, -1, +1 ),Colors::Blue},
		{Vector3( +1, -1, +1 ),Colors::Magenta},
		{Vector3( -1, +1, +1 ),Colors::Cyan},
		{Vector3( +1, +1, +1 ),Colors::White},
	};

	unsigned dataBytes=sizeof(Vertex)*ARRAYSIZE(v);	// size of vertex buffer in bytes

	// create the vertex buffer
	mpVertexBuffer=CreateVertexBuffer(GetDevice(),v,dataBytes);

	UINT indices[]=
	{
		0,3,1,	// Front
		0,2,3,	
		1,7,5,	// Right
		1,3,7,	
		0,4,2,	// Left
		6,2,4,	
		7,6,5,	// back
		4,5,6,	
		2,7,3,	// top
		2,6,7,	
		0,1,5,	// bottom
		0,5,4,	
	};
	mNumIndex=ARRAYSIZE(indices);	// number of indexes (needed for rendering)

	// create index buffer
	mpIndexBuffer=CreateIndexBuffer(GetDevice(),indices,mNumIndex);

	// don't need create shaders, input layout & constant buffers
	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));

	// initial pos/tgt for camera
	mCamera.Reset();
}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();

	if (Input::KeyPress(VK_F1))
	{
		GetContext()->RSSetState(mpShaderManager->CommonStates()->CullNone());
		mTitle=L"Rasterizer state: CullNone";
	}
	if (Input::KeyPress(VK_F2))
	{
		GetContext()->RSSetState(mpShaderManager->CommonStates()->CullClockwise());
		mTitle=L"Rasterizer state: CullClockwise";
	}
	if (Input::KeyPress(VK_F3))
	{
		GetContext()->RSSetState(mpShaderManager->CommonStates()->CullCounterClockwise());
		mTitle=L"Rasterizer state: CullCounterClockwise";
	}
	if (Input::KeyPress(VK_F4))
	{
		GetContext()->RSSetState(mpShaderManager->CommonStates()->Wireframe());
		mTitle=L"Rasterizer state: Wireframe";
	}

	// move the camera
	mCamera.Update();
}

void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(), Colors::SkyBlue);

	// Set Vertex and Pixel Shaders
	
	
	//GetContext()->PSSetShader(mpShaderManager->PSDummyLight() , 0, 0);


	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Set the Input Layout
	GetContext()->IASetInputLayout( mpShaderManager->mpLayoutDefault );

	// Set Vertex and Pixel Shaders
	//GetContext()->VSSetShader(mpShaderManager->mpVSColoured , 0, 0);
	//GetContext()->PSSetShader(mpShaderManager->mpPSColoured , 0, 0);

	GetContext()->VSSetShader(mpShaderManager->VSDefault() , 0, 0);
	GetContext()->PSSetShader(mpShaderManager->PSUnlit() , 0, 0);

	// Set the vertex/index buffer
	ShaderManager::SetVertexIndexBuffers(GetContext(),mpVertexBuffer,sizeof(ColouredVertex),mpIndexBuffer);

	// fill constant buffer with WVP matrix
	Matrix world=Matrix::Identity(); //Matrix::CreateRotationY(Timer::GetTime());
	Matrix WVP=world*mCamera.GetViewMatrix()*mCamera.GetDefaultProjectionMatrix();
	ShaderManager::cbMatrixBasic cbPerObj;
	cbPerObj.gWorldViewProj = WVP.Transpose();		// NB: You must transpose for the shaders
	
	// set constant buffer
	ShaderManager::SetVSConstantBuffer(GetContext(),mpShaderManager->CBMatrixInfo(),&cbPerObj);
	//ShaderManager::SetPSConstantBuffer(GetContext(),mpShaderManager->CBMaterial(),&cbPerObj);

	

	// do the actual draw
	GetContext()->DrawIndexed(mNumIndex,0,0);	// number of indexes, not shapes

	// set buffers to something else:
	world=Matrix::CreateRotationY(Timer::GetTime())*Matrix::CreateTranslation(2,2,2);
	WVP=world*mCamera.GetViewMatrix()*mCamera.GetDefaultProjectionMatrix();
	cbPerObj.gWorldViewProj = WVP.Transpose();		// NB: You must transpose for the shaders
	// set constant buffer
	ShaderManager::SetVSConstantBuffer(GetContext(),mpShaderManager->CBMatrixBasic(),&cbPerObj);
	// do the actual draw
	GetContext()->DrawIndexed(mNumIndex,0,0);	// number of indexes, not shapes

	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);
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
