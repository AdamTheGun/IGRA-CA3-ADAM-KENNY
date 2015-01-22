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
#include "SpriteBatch.h" 
#include "SpriteFont.h"

#include "Vertex.h"	// the IGRA vertex structures

#include "GeometryGenerator.h"	// the IGRA vertex structures
# define M_PI           3.14159265358979323846  /* pi */
#define D3DXToRadian(degree) ((degree) * (M_PI / 180.0f))

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
	
	BasicMaterial mBasicMaterial;
	SimpleMesh mGround,mBox,mCarWindow,mBuilding,mSidewalk1,mFireHydrant,mWaterSpray;
	CComPtr<ID3D11ShaderResourceView> mpTexLogo,mpRoadTex,mpBrokenGlass,mpBuildingTex,mpSidewalk,mpFireHydrant,mpWaterTex;
	std::string nameidStr;

	ArcBallCamera mCamera;

	Vector3 CarPos;
	Vector3 WaterScale,WaterScale2;
	Vector3 Campos1,CamPos2,Camtgt1,Camtgt2;
	Vector3 CurrCamPos,CurrCamTgt;
	float CarSpeed;
	float CarRotation;

	std::unique_ptr<SpriteBatch> mpSpriteBatch;
	std::unique_ptr<SpriteFont> mpSpriteFont;

	void DrawSimpleMesh(SimpleMesh& mesh);
	//void CreateBuilding(float,float,float,ID3D11Device dev,SimpleMesh,GeometryGenerator,GeometryGenerator::MeshData);

	/*
	ShaderManager::cbMaterial cbMat;
	ShaderManager::cbLights cbLight;
	void SetupLights();
	void SetVertCBuffer(Matrix& world,Matrix& view, Matrix& proj);
	void SetPixCBuffer(Material& mat);
	*/
};

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 5a: Textures";

	CarSpeed = 0.0f;
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	

	std::wstring useThis =L"Hello World";

	mpSpriteBatch.reset(new SpriteBatch(GetContext()));
	mpSpriteFont.reset(new SpriteFont(GetDevice(), L"../Content/Times12.sprfont"));

	GeometryGenerator generator;
	GeometryGenerator::MeshData mesh;

	// make a grid
	generator.CreateGrid(500,500,2,2,mesh);
	//print some data
	DebugLog("Grid Data \n");

	CarPos = Vector3(0,1,0);
	WaterScale = Vector3(1,1,1);
	WaterScale2 = Vector3(1,5,1);
	Campos1 = Vector3(5,1,46);
	Camtgt1 = Vector3(0,0.5,-1);
	CamPos2 = Vector3(14,1,-25);
	Camtgt2 = Vector3(0,0.5,3);
	CurrCamPos = Campos1;
	CurrCamTgt = Camtgt1;
	CarRotation = 0.0f;

	for(unsigned int i = 0;i<mesh.Vertices.size();i++)
	{
		DebugLog(ToString("Vertex: ",i," pos ",mesh.Vertices[i].Position));
		DebugLog(ToString(" tex: ",mesh.Vertices[i].TexC,"\n"));
	}

	mesh.Vertices[0].TexC=Vector2(0,0);
	mesh.Vertices[1].TexC=Vector2(50,0);
	mesh.Vertices[2].TexC=Vector2(0,50);
	mesh.Vertices[3].TexC=Vector2(50,50);

	 //convert to vertex buffer,index buffer
	mGround.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mGround.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mGround.mNumIndex=mesh.Indices.size();
	mGround.mMaterial=MakeMaterial(Colors::White,Colors::Black);
	
	generator.CreateBox(16,18,32,mesh);
	mBuilding.mpVertexBuffer = CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex)*mesh.Vertices.size());
	mBuilding.mpIndexBuffer = CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mBuilding.mNumIndex=mesh.Indices.size();
	mBuilding.mMaterial=MakeMaterial(Colors::Gray,Colors::Black);

	generator.CreateBox(16,1,32,mesh);
	mSidewalk1.mpVertexBuffer = CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex)*mesh.Vertices.size());
	mSidewalk1.mpIndexBuffer = CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mSidewalk1.mNumIndex=mesh.Indices.size();
	mSidewalk1.mMaterial=MakeMaterial(Colors::Gray,Colors::Black);

	/*
	generator.CreateBox(2,18,32,mesh);
	mScreen.mpVertexBuffer = CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex)*mesh.Vertices.size());
	mScreen.mpIndexBuffer = CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mScreen.mNumIndex=mesh.Indices.size();
	mScreen.mMaterial=MakeMaterial(Colors::Gray,Colors::Black);

	

	generator.CreateBox(50,50,50,mesh);
	mRoom.mpVertexBuffer = CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex)*mesh.Vertices.size());
	mRoom.mpIndexBuffer = CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mRoom.mNumIndex=mesh.Indices.size();
	mRoom.mMaterial=MakeMaterial(Colors::Gray,Colors::Black);

	
	generator.CreateBox(0.3,3,1.5,mesh);
	mChairBack.mpVertexBuffer = CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex)*mesh.Vertices.size());
	mChairBack.mpIndexBuffer = CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mChairBack.mNumIndex=mesh.Indices.size();
	mChairBack.mMaterial=MakeMaterial(Colors::Gray,Colors::Black);

	generator.CreateBox(1,1.5,1.5,mesh);
	mChairSeat.mpVertexBuffer = CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex)*mesh.Vertices.size());
	mChairSeat.mpIndexBuffer = CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mChairSeat.mNumIndex=mesh.Indices.size();
	mChairSeat.mMaterial=MakeMaterial(Colors::Gray,Colors::Black);
	*/

	// now a box
	generator.CreateBox(1,3,1,mesh);
	// convert to vertex buffer,index buffer
	mFireHydrant.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mFireHydrant.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mFireHydrant.mNumIndex=mesh.Indices.size();
	mFireHydrant.mMaterial=MakeMaterial(Colors::White,Colors::Red,16.0f);


	// now a box
	generator.CreateBox(3,1,5,mesh);
	// convert to vertex buffer,index buffer
	mBox.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mBox.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mBox.mNumIndex=mesh.Indices.size();
	mBox.mMaterial=MakeMaterial(Colors::White,Colors::Black);

	generator.CreateSphere(1,16,10,mesh);
	// convert to vertex buffer,index buffer
	mCarWindow.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mCarWindow.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mCarWindow.mNumIndex=mesh.Indices.size();
	mCarWindow.mMaterial=MakeMaterial(Colors::White,Colors::White,2);

	generator.CreateCylinder(0.5f,2,4,6,6,mesh);
	// convert to vertex buffer,index buffer
	mWaterSpray.mpVertexBuffer=CreateVertexBuffer(GetDevice(),&mesh.Vertices[0],sizeof(GeometryGenerator::Vertex) * mesh.Vertices.size());
	mWaterSpray.mpIndexBuffer=CreateIndexBuffer(GetDevice(),&mesh.Indices[0],mesh.Indices.size());
	mWaterSpray.mNumIndex=mesh.Indices.size();
	mWaterSpray.mMaterial=MakeMaterial(Colors::Blue,Colors::SkyBlue,6);

	// just create the Shader Manager
	

		mBasicMaterial.mpShaderManager=mpShaderManager.get();
	mBasicMaterial.mpLayout=mpShaderManager->LayoutDefault();
	mBasicMaterial.mpVS=mpShaderManager->VSDefault();
	mBasicMaterial.mpPS=mpShaderManager->PSDirLight();
	mBasicMaterial.SetupDefaultLights();	

	mpBrokenGlass.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/BrokenGlass_Tex.bmp"));
	mpTexLogo.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/directx-11-logo.png"));
	mpRoadTex.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/asphalt.jpg"));
	mpBuildingTex.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/Building.jpg"));
	mpSidewalk.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/sidewalk.jpg"));
	mpFireHydrant.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/Red.jpg"));
	mpWaterTex.Attach(CreateTextureResourceWIC(GetDevice(),L"../Content/Water.jpg"));



	// initial pos/tgt for camera
	mCamera.Reset();

}


void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	CarPos.z +=CarSpeed*Timer::GetDeltaTime();
	if(Input::KeyDown('W'))
	{
		CarSpeed += 3*Timer::GetDeltaTime();
	}

	if(Input::KeyPress(VK_F1))
	{
		CurrCamPos = Campos1;
		CurrCamTgt = Camtgt1;
	}

	if(Input::KeyPress(VK_F2))
	{
		CurrCamPos = CamPos2;
		CurrCamTgt = Camtgt2;
	}

	WaterScale.y+=5*Timer::GetDeltaTime();
	WaterScale2.y+=5*Timer::GetDeltaTime();
	if(WaterScale.y>=5)
	{
		WaterScale.y = 1;
	}
	if(WaterScale2.y>=6)
	{
		WaterScale2.y =1;
	}
	if(CarSpeed>0)
	{
		CarSpeed-=1*Timer::GetDeltaTime();
	}
	if(CarSpeed<0)
	{
		CarSpeed+=1*Timer::GetDeltaTime();
	}

	if(Input::KeyDown('S'))
	{
		CarSpeed-= 3*Timer::GetDeltaTime();
	}
	if(Input::KeyDown('A'))
	{
		CarRotation -= 3*Timer::GetDeltaTime();
	}
	if(Input::KeyDown('D'))
	{
		CarRotation += 3*Timer::GetDeltaTime();
	}
	
	// move the camera
	mCamera.Update();
}

void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(), Colors::DarkBlue);
	GetContext()->OMSetDepthStencilState(mpShaderManager->CommonStates()->DepthDefault(),0);


	// turn off culling, so we can see the back of the grids
	GetContext()->RSSetState(mpShaderManager->CommonStates()->CullNone());

	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Set the Input Layout
	//GetContext()->IASetInputLayout( mpShaderManager->LayoutDefault() );

	// Set Vertex and Pixel Shaders
	//GetContext()->VSSetShader(mpShaderManager->VSDefault() , 0, 0);
	//GetContext()->PSSetShader(mpShaderManager->PSDirLight() ,0, 0);
	//GetContext()->PSSetShader(mpShaderManager->PSDirLight() , 0, 0);

	mBasicMaterial.SetShaderLayout(GetContext());

	//cbLight.gEyePosW=Campos1;

	// set the shader resource (texture)
	//ShaderManager::SetShaderResourceSampler(GetContext(),mpTexLogo)
	
	// set a sampler: try uncommenting one of these
	ShaderManager::SetSampler(GetContext(),
		mpShaderManager->CommonStates()->AnisotropicWrap());
	//ShaderManager::SetSampler(GetContext(),
                     //mpShaderManager->CommonStates()->LinearClamp());
	//ShaderManager::SetSampler(GetContext(),
                   // mpShaderManager->CommonStates()->AnisotropicClamp());

	// fill constant buffer with WVP matrix
	Matrix world;
	Matrix view=XMMatrixLookAtLH(CurrCamPos,CurrCamTgt,Vector3(0,1,0));
	Matrix proj=XMMatrixPerspectiveFovLH(0.4f*M_PI,GetAspectRatio(),0.1f,200);
	mBasicMaterial.FillMatrixes(world,view,proj);
	



	//ShaderManager::SetShaderResource(GetContext(),mpTexLogo);
	ShaderManager::SetShaderResource(GetContext(),mpBuildingTex);
	//Building
	world=Matrix::CreateScale(1,1.2,1)*Matrix::CreateTranslation(-20,9000,0);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mBuilding.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mBuilding);

	ShaderManager::SetShaderResource(GetContext(),mpSidewalk);
	//Sidewalk1
	world=Matrix::CreateScale(1.6,1,1.3)*Matrix::CreateTranslation(-19,0,0);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mSidewalk1.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mSidewalk1);


	
	
		//Building
	ShaderManager::SetShaderResource(GetContext(),mpBuildingTex);
	world=Matrix::CreateScale(1,1.2,1)*Matrix::CreateTranslation(-20,9,40);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mBuilding.mMaterial;
	mBasicMaterial.Apply(GetContext());
	DrawSimpleMesh(mBuilding);

	//Sidewalk1
	ShaderManager::SetShaderResource(GetContext(),mpSidewalk);
	world=Matrix::CreateScale(1.6,1,1.3)*Matrix::CreateTranslation(-19,0,40);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mSidewalk1.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mSidewalk1);

	ShaderManager::SetShaderResource(GetContext(),mpBuildingTex);
	world=Matrix::CreateScale(1,1.2,1)*Matrix::CreateTranslation(25,9,0);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mBuilding.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mBuilding);

	ShaderManager::SetShaderResource(GetContext(),mpSidewalk);
	world=Matrix::CreateScale(1.6,1,1.3)*Matrix::CreateTranslation(24,0,0);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mSidewalk1.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mSidewalk1);

	ShaderManager::SetShaderResource(GetContext(),mpBuildingTex);
	world=Matrix::CreateScale(1,1.2,1)*Matrix::CreateTranslation(25,9,40);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mBuilding.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mBuilding);

	ShaderManager::SetShaderResource(GetContext(),mpSidewalk);
	world=Matrix::CreateScale(1.6,1,1.3)*Matrix::CreateTranslation(24,0,40);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mSidewalk1.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mSidewalk1);

	ShaderManager::SetShaderResource(GetContext(),mpBuildingTex);
	world=Matrix::CreateScale(1,1.2,1)*Matrix::CreateRotationY(D3DXToRadian(90))*Matrix::CreateTranslation(25,9,-50);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mBuilding.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mBuilding);
	
	ShaderManager::SetShaderResource(GetContext(),mpSidewalk);
	world=Matrix::CreateScale(1.6,1,1.5)*Matrix::CreateRotationY(D3DXToRadian(90))*Matrix::CreateTranslation(24,0,-50);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mSidewalk1.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mSidewalk1);

	ShaderManager::SetShaderResource(GetContext(),mpBuildingTex);
	world=Matrix::CreateScale(1,2.6,1)*Matrix::CreateRotationY(D3DXToRadian(90))*Matrix::CreateTranslation(-25,19,-50);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mBuilding.mMaterial;
	mBasicMaterial.Apply(GetContext());

	DrawSimpleMesh(mBuilding);
	
	ShaderManager::SetShaderResource(GetContext(),mpSidewalk);
	world=Matrix::CreateScale(1.6,1,1.3)*Matrix::CreateRotationY(D3DXToRadian(90))*Matrix::CreateTranslation(-24,0,-50);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mSidewalk1.mMaterial;
	mBasicMaterial.Apply(GetContext());
	
	DrawSimpleMesh(mSidewalk1);

	ShaderManager::SetShaderResource(GetContext(),mpFireHydrant);
	world=Matrix::CreateScale(1,1,1)*Matrix::CreateTranslation(15,1,-15);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mFireHydrant.mMaterial;
	mBasicMaterial.Apply(GetContext());
	
	DrawSimpleMesh(mFireHydrant);


	world=Matrix::CreateRotationY(CarRotation)*Matrix::CreateTranslation(CarPos);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mBox.mMaterial;
	mBasicMaterial.Apply(GetContext());
	
	DrawSimpleMesh(mBox);

	ShaderManager::SetShaderResource(GetContext(),mpBrokenGlass);
	// sphere
	world=Matrix::CreateScale(1,1,2)*Matrix::CreateRotationY(CarRotation)*Matrix::CreateTranslation(CarPos.x,CarPos.y+0.5,CarPos.z);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mCarWindow.mMaterial;
	mBasicMaterial.Apply(GetContext());
	
	DrawSimpleMesh(mCarWindow);
	
	ShaderManager::SetShaderResource(GetContext(),mpWaterTex);
	world=Matrix::CreateTranslation(15,2.5,-15)*Matrix::CreateScale(WaterScale);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mWaterSpray.mMaterial;
	mBasicMaterial.Apply(GetContext());
	
	DrawSimpleMesh(mWaterSpray);
	
	ShaderManager::SetShaderResource(GetContext(),mpWaterTex);
	world=Matrix::CreateTranslation(15,2.5,-15)*Matrix::CreateScale(WaterScale2);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mWaterSpray.mMaterial;
	mBasicMaterial.Apply(GetContext());
	DrawSimpleMesh(mWaterSpray);

	ShaderManager::SetShaderResource(GetContext(),mpRoadTex);
	// draw ground:
	world=Matrix::CreateTranslation(0,-0.7,0);
	mBasicMaterial.FillMatrixes(world,view,proj);
	mBasicMaterial.mMaterial.gMaterial = mGround.mMaterial;
	mBasicMaterial.Apply(GetContext());
	
	DrawSimpleMesh(mGround);

	mpSpriteBatch->Begin();
	std::wstring str = ToString("NAME: MUHD ADAM BIN MOHAMAD AMIN\nADMIN NO: 1333859");

	mpSpriteFont->DrawString(mpSpriteBatch.get(),str.c_str(),Vector2(20,20),Colors::White);
    mpSpriteBatch->End();
	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);
}
//
//void MyApp::SetVertCBuffer(Matrix& world,Matrix& view, Matrix& proj)
//{
//	ShaderManager::cbMatrixInfo cbMI;
//	cbMI.gWorldViewProj=(world*view*proj).Transpose();		// NB: You must transpose for the shaders
//	cbMI.gWorld=world.Transpose();
//	cbMI.gWorldInvTranspose=world.Invert();	// the 2 transposes cancel
//	
//	// set constant buffer
//	ShaderManager::SetVSConstantBuffer(GetContext(),mpShaderManager->CBMatrixInfo(),&cbMI);
//}
//
//void MyApp::SetPixCBuffer(Material& mat)
//{
//	/*
//	ShaderManager::cbMaterial cbMat;
//	ZeroMemory(&cbMat,sizeof(cbMat));
//	cbMat.gMaterial=mat;
//	cbMat.gUseTexture=true;
//	// set constant buffer
//	ShaderManager::SetPSConstantBuffer(GetContext(),mpShaderManager->CBLights(),&cbMat);
//	*/
//	cbMat.gMaterial=mat;
//
//	// set constant buffers
//	GetContext()->UpdateSubresource( mpShaderManager->CBMaterial(), 0, NULL,
//                                           &cbMat, 0, 0 );
//	GetContext()->UpdateSubresource( mpShaderManager->CBLights(), 0, NULL,
//                                           &cbLight, 0, 0 );
//	ID3D11Buffer* buffers[]={mpShaderManager->CBMaterial(),
//                                mpShaderManager->CBLights()};
//	// set 2  buffers (b0 & b1)
//	GetContext()->PSSetConstantBuffers(0, 2, buffers); 
//
//}

void MyApp::DrawSimpleMesh(SimpleMesh& mesh)
{
	// fill the pixel shader info:

	// Set the vertex/index buffer
	ShaderManager::SetVertexIndexBuffers(GetContext(),mesh.mpVertexBuffer,sizeof(Vertex),mesh.mpIndexBuffer);
	// do the actual draw
	GetContext()->DrawIndexed(mesh.mNumIndex,0,0);	// number of indexes, not shapes
}

//
//void MyApp::SetupLights()
//{
//	
//	cbLight.gDirLights[0].Ambient  = Color(0.3f, 0.3f, 0.3f, 1.0f);
//	cbLight.gDirLights[0].Diffuse  = Color(0.0f, 0.0f, 0.0f, 1.0f);
//	cbLight.gDirLights[0].Specular = Color(0.3f, 0.3f, 0.3f, 1.0f);
//	cbLight.gDirLights[0].Direction = Vector3(0.57735f, -0.97735f, 0.57735f);
//	
//	cbLight.gDirLights[1].Ambient  = Color(0.1f, 0.1f, 0.1f, 1.0f);
//	cbLight.gDirLights[1].Diffuse  = Color(0.0f,0.0f,0.0f, 1.0f);
//	cbLight.gDirLights[1].Specular = Color(0.15f, 0.15f, 0.15f, 1.0f);
//	cbLight.gDirLights[1].Direction = Vector3(0.57735f,0.57735f,0.57735f);
//	
//	cbLight.gDirLights[2].Ambient  = Color(0.0f, 0.0f, 0.0f, 1.0f);
//	cbLight.gDirLights[2].Diffuse  = Color(0.992f, 0.721f, 0.074f, 1.0f);
//	cbLight.gDirLights[2].Specular = Color(0.3f, 0.3f, 0.3f, 1.0f);
//	cbLight.gDirLights[2].Direction = Vector3(0.0f, -0.707f, -0.707f);
//}


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
