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
	Material mMaterial;

	ShaderManager::cbMaterial cbMat;
	ShaderManager::cbLights cbLight;
	void SetupLights();
	void SetPixCBuffer(Material& mat);

	ArcBallCamera mCamera;
};

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 4a: The Cubes, with Shader Manager";

	//Create the vertex buffer
	GeometryGenerator generator;
	GeometryGenerator::MeshData mesh;
	//generator.CreateBox(1,1,1,mesh);
	generator.CreateSphere(1,12,12,mesh);

	// convert to vertex buffer,index buffer
	mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mNumIndex=mesh.Indices.size();
	mMaterial=MakeMaterial(Colors::LightPink,Colors::Black);

	// don't need create shaders, input layout & constant buffers
	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	SetupLights();
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

	//GetContext()->RSSetState(mpShaderManager->CommonStates()->CullNone());

	
	//GetContext()->PSSetShader(mpShaderManager->PSDummyLight() , 0, 0);


	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Set the Input Layout
	GetContext()->IASetInputLayout( mpShaderManager->LayoutDefault() );

	// Set Vertex and Pixel Shaders
	//GetContext()->VSSetShader(mpShaderManager->mpVSColoured , 0, 0);
	//GetContext()->PSSetShader(mpShaderManager->mpPSColoured , 0, 0);

	GetContext()->VSSetShader(mpShaderManager->VSDefault() , 0, 0);
	//GetContext()->PSSetShader(mpShaderManager->PSUnlit() , 0, 0);
	//GetContext()->PSSetShader(mpShaderManager->PSDummyLight() , 0, 0);

	GetContext()->PSSetShader(mpShaderManager->PSDirLight() , 0, 0);
	// Set the vertex/index buffer
	ShaderManager::SetVertexIndexBuffers(GetContext(),mpVertexBuffer,sizeof(Vertex),mpIndexBuffer);

	// fill constant buffer with WVP matrix
	Matrix world=Matrix::Identity(); //Matrix::CreateRotationY(Timer::GetTime());
	Matrix WVP=world*mCamera.GetViewMatrix()*mCamera.GetDefaultProjectionMatrix();
	ShaderManager::cbMatrixInfo cbPerObj;
	cbPerObj.gWorldViewProj = WVP.Transpose();		// NB: You must transpose for the shaders
	cbPerObj.gWorld=world.Transpose();
	cbPerObj.gWorldInvTranspose=world.Invert();
	
	// set constant buffer
	ShaderManager::SetVSConstantBuffer(GetContext(),mpShaderManager->CBMatrixInfo(),&cbPerObj);

	// now for the material (pixel shader)
	ShaderManager::cbMaterial cbMat;
	cbMat.gMaterial=mMaterial;
	ShaderManager::SetPSConstantBuffer(GetContext(),mpShaderManager->CBMaterial(),&cbMat);

	

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

void MyApp::SetupLights()
{
	cbLight.gDirLights[0].Ambient  = Color(0.2f, 0.2f, 0.2f, 1.0f);
	cbLight.gDirLights[0].Diffuse  = Color(0.5f, 0.5f, 0.5f, 1.0f);
	cbLight.gDirLights[0].Specular = Color(0.3f, 0.3f, 0.3f, 1.0f);
	cbLight.gDirLights[0].Direction = Vector3(0.57735f, -0.97735f, 0.57735f);

	cbLight.gDirLights[1].Ambient  = Color(0.0f, 0.0f, 0.0f, 1.0f);
	cbLight.gDirLights[1].Diffuse  = Color(0.20f, 0.20f, 0.20f, 1.0f);
	cbLight.gDirLights[1].Specular = Color(0.15f, 0.15f, 0.15f, 1.0f);
	cbLight.gDirLights[1].Direction = Vector3(-0.57735f,-0.57735f,0.57735f);

	cbLight.gDirLights[2].Ambient  = Color(0.0f, 0.0f, 0.0f, 1.0f);
	cbLight.gDirLights[2].Diffuse  = Color(0.2f, 0.2f, 0.2f, 1.0f);
	cbLight.gDirLights[2].Specular = Color(0.0f, 0.0f, 0.0f, 1.0f);
	cbLight.gDirLights[2].Direction = Vector3(0.0f, -0.707f, -0.707f);
}


void MyApp::SetPixCBuffer(Material& mat)
{
	cbMat.gMaterial=mat;

	// set constant buffers
	GetContext()->UpdateSubresource( mpShaderManager->CBMaterial(), 0, NULL,
                                           &cbMat, 0, 0 );
	GetContext()->UpdateSubresource( mpShaderManager->CBLights(), 0, NULL,
                                           &cbLight, 0, 0 );
	ID3D11Buffer* buffers[]={mpShaderManager->CBMaterial(),
                                mpShaderManager->CBLights()};
	// set 2  buffers (b0 & b1)
	GetContext()->PSSetConstantBuffers(0, 2, buffers); 
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
