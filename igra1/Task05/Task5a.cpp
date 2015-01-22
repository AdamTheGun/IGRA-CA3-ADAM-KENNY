/******************************
* Task5a.cpp
* Written by The IGRA team
******************************/

#include "DxCommon.h"	// common DX stuff
#include <DirectXColors.h>	// DirectX::Colors

#include "IgraApp.h"	// the IGRA 'game' class
#include "IgraUtils.h"	// useful functions
#include "ShaderManager.h"	// holds all the shaders
#include "ToString.h"
#include "SimpleMathToString.h"

#include "Vertex.h"	// the IGRA vertex structures

#include "GeometryGenerator.h"	// the IGRA vertex structures

using namespace igra;
using namespace DirectX;	// XMVECTOR
using namespace DirectX::SimpleMath;	// for Color

struct SimpleMesh
{
	CComPtr<ID3D11Buffer> mpVertexBuffer;	// vertex buffer
	CComPtr<ID3D11Buffer> mpIndexBuffer;	// index buffer
	unsigned mNumIndex;						// number of indexes
	Material mMaterial;
};

class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	//void Shutdown();	

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;

	SimpleMesh mGround,mBox,mBoard;
	CComPtr<ID3D11ShaderResourceView> mpTexLogo;

	ArcBallCamera mCamera;

	void SetVertCBuffer(Matrix& world,Matrix& view, Matrix& proj);
	void SetPixCBuffer(Material& mat);
	void DrawSimpleMesh(SimpleMesh& mesh);

	ShaderManager::cbMaterial cbMat;
	ShaderManager::cbLights cbLight;
	void SetupLights();

};

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 5a: Textures";


	SetupLights();

	GeometryGenerator generator;
	GeometryGenerator::MeshData mesh;

	// make a grid
	generator.CreateGrid(10,10,5,5,mesh);
	// convert to vertex buffer,index buffer
	mGround.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mGround.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mGround.mNumIndex=mesh.Indices.size();
	mGround.mMaterial=MakeMaterial(Colors::LawnGreen,Colors::Black);

	// now a box
	generator.CreateBox(1,1,1,mesh);
	// convert to vertex buffer,index buffer
	mBox.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mBox.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mBox.mNumIndex=mesh.Indices.size();
	mBox.mMaterial=MakeMaterial(Colors::LightPink,Colors::Black);

	// finally a board
	generator.CreateGrid(2,2,2,2,mesh);
	// convert to vertex buffer,index buffer
	mBoard.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mBoard.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mBoard.mNumIndex=mesh.Indices.size();
	mBoard.mMaterial=MakeMaterial(Colors::White,Colors::Black);

	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));

	mpTexLogo.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/directx-11-logo.png"));

	// initial pos/tgt for camera
	mCamera.Reset();

}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	// move the camera
	mCamera.Update();
}

void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(), Colors::SkyBlue);

	// turn off culling, so we can see the back of the grids
	GetContext()->RSSetState(mpShaderManager->CommonStates()->CullNone());

	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Set the Input Layout
	GetContext()->IASetInputLayout( mpShaderManager->LayoutDefault() );

	// Set Vertex and Pixel Shaders
	GetContext()->VSSetShader(mpShaderManager->VSDefault() , 0, 0);
	GetContext()->PSSetShader(mpShaderManager->PSUnlit() , 0, 0);

	// set the shader resource (texture)
	//ShaderManager::SetShaderResourceSampler(GetContext(),mpTexLogo)
	ShaderManager::SetShaderResource(GetContext(),mpTexLogo);
	// set a sampler: try uncommenting one of these
	//ShaderManager::SetSampler(GetContext(),
       //             mpShaderManager->CommonStates()->PointClamp());
	//ShaderManager::SetSampler(GetContext(),
       //              mpShaderManager->CommonStates()->LinearClamp());
	ShaderManager::SetSampler(GetContext(),
                    mpShaderManager->CommonStates()->AnisotropicClamp());

	// fill constant buffer with WVP matrix
	Matrix world;
	Matrix view=mCamera.GetViewMatrix();
	Matrix proj=mCamera.GetDefaultProjectionMatrix();

	// draw ground:
	world=Matrix::CreateTranslation(0,-1,0);
	SetVertCBuffer(world,view,proj);
	SetPixCBuffer(mGround.mMaterial);
	DrawSimpleMesh(mGround);

	// box
	world=Matrix::CreateRotationY(Timer::GetTime());
	SetVertCBuffer(world,view,proj);
	SetPixCBuffer(mBox.mMaterial);
	DrawSimpleMesh(mBox);

	// sphere
	world=Matrix::CreateRotationX(-XM_PIDIV2)*Matrix::CreateTranslation(2,0,0);
	SetVertCBuffer(world,view,proj);
	SetPixCBuffer(mBoard.mMaterial);
	DrawSimpleMesh(mBoard);

	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);
}

void MyApp::SetVertCBuffer(Matrix& world,Matrix& view, Matrix& proj)
{
	ShaderManager::cbMatrixInfo cbMI;
	cbMI.gWorldViewProj=(world*view*proj).Transpose();		// NB: You must transpose for the shaders
	cbMI.gWorld=world.Transpose();
	cbMI.gWorldInvTranspose=world.Invert();	// the 2 transposes cancel
	
	// set constant buffer
	ShaderManager::SetVSConstantBuffer(GetContext(),mpShaderManager->CBMatrixInfo(),&cbMI);
}

void MyApp::SetPixCBuffer(Material& mat)
{
	ShaderManager::cbMaterial cbMat;
	ZeroMemory(&cbMat,sizeof(cbMat));
	cbMat.gMaterial=mat;
	cbMat.gUseTexture=true;
	// set constant buffer
	ShaderManager::SetPSConstantBuffer(GetContext(),mpShaderManager->CBMaterial(),&cbMat);
}

void MyApp::DrawSimpleMesh(SimpleMesh& mesh)
{
	// fill the pixel shader info:

	// Set the vertex/index buffer
	ShaderManager::SetVertexIndexBuffers(GetContext(),mesh.mpVertexBuffer,sizeof(Vertex),mesh.mpIndexBuffer);
	// do the actual draw
	GetContext()->DrawIndexed(mesh.mNumIndex,0,0);	// number of indexes, not shapes
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
