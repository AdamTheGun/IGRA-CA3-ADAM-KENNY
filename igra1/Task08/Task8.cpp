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
#include "Node.h"
#include "Vertex.h"	// the IGRA vertex structures
#include "Terrain.h"

#include "GeometryGenerator.h"	// the IGRA vertex structures
# define M_PI           3.14159265358979323846  /* pi */
#define D3DXToRadian(degree) ((degree) * (M_PI / 180.0f))
#include "ModelObj.h"	// the IGRA model loader

using namespace DirectX::Colors;
using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace igra;

class Bullet : public DrawableNode
{
public:
	Vector3 mVelocity;
	float mLifetime;
	virtual void Update();
};

class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	void Shutdown();	
	void UpdateCamera();

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;
	std::unique_ptr<PrimitiveBatch<ColouredVertex>> mpDraw3D;
	
	ModelObj mTeapot,mPlane,mAxisObj,mBox,mYellowBallObj,mCarShipObj;
	

	std::vector<DrawableNode*> mBoxes;
	std::vector<Bullet*> mBullets;
	DrawableNode mGround,mAxis,mPlayer,mWatcher,mCarShip;
	bool mDiagnostics;
	void FireShot();
	void CheckCollisions();

	bool mNodeCamera;
	
	ArcBallCamera mCamera;
	std::unique_ptr<Terrain> mpTerrain;
	TerrainMaterial mTerrainMat;

};



void Bullet::Update()
{
	mLifetime-= Timer::GetDeltaTime();
	mPos+=mVelocity*Timer::GetDeltaTime();
	if(mLifetime<0)
	{
		Kill();//Kills self
	}
}
void MyApp::FireShot()
{
	Bullet* ptr = FindDeadNode(mBullets);
	if(ptr== nullptr)return;

	ptr->mHealth=100;
	ptr->mLifetime= 3.0f;
	ptr->SetPos(mPlayer.GetPos()+mPlayer.RotateVector(Vector3(0,0.45f,1.2f)));
	ptr->mVelocity = mPlayer.RotateVector(Vector3(0,0,10));
	ptr->mScale=0.2f;
}
void MyApp::CheckCollisions()
{
	for(int b = 0;b<mBullets.size();b++)
	{
		if(mBullets[b]->IsAlive()==false) continue;
		
		BoundingSphere bs = mBullets[b]->GetBounds();

		for(int t = 0;t <mBoxes.size();t++)
		{
			if(mBoxes[t]->IsAlive()==false)continue;

			if(bs.Intersects(mBoxes[t]->GetBounds()))
			{
				mBullets[b]->Kill();
				mBoxes[t]->mHealth-=25;
			}
		}
	}
}

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 8a: Terrain";

	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	
	// initial pos/tgt for camera
	//mCamera.Reset();


	mpDraw3D.reset(new PrimitiveBatch<ColouredVertex>(GetContext()));

	// load models:
	mTeapot.Load(GetDevice(),mpShaderManager.get(),L"../Content/teapot.obj",true);
	mPlane.Load(GetDevice(),mpShaderManager.get(),L"../Content/plane.obj",true);
	mAxisObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/axis1.obj");
	mBox.Load(GetDevice(),mpShaderManager.get(),L"../Content/crate.obj");
	mYellowBallObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/yellow_ball.obj");
	mCarShipObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/Car-Ship.obj");
	
	mDiagnostics = false;

	mGround.Init(&mPlane,Vector3(0,0,0));
	mAxis.Init(&mAxisObj);
	mPlayer.Init(&mTeapot,Vector3(0,25,0));
	mWatcher.Init(&mTeapot,Vector3(0,2,1));
	mCarShip.Init(&mCarShipObj,Vector3(2,2,2));
	mCarShip.mScale = 0.01;
	
	mCamera.Reset();
	
	
	mpTerrain.reset(new Terrain(GetDevice(),L"../Content/Terrain/igra.bmp",
                            Vector3(1,0.1f,1)));
	mTerrainMat.mpShaderManager=mpShaderManager.get();
	mTerrainMat.mpTexBase.Attach(CreateTextureResourceWIC(GetDevice(),
                          L"../Content/Terrain/igra_d.png"));

	std::vector<std::wstring> textures;
	textures.push_back(L"../Content/Terrain/Sandripple.png");
	textures.push_back(L"../Content/Terrain/bigRockFace.png");
	textures.push_back(L"../Content/Terrain/longGrass.png");
	textures.push_back(L"../Content/Terrain/brownSand.png");

	mTerrainMat.mpTexLayerArray.Attach(CreateTexture2DArray(GetDevice(),GetContext(),textures));
	mTerrainMat.mpTexLight.Attach(CreateTextureResourceWIC(
                               GetDevice(),L"../Content/Terrain/igra_l.png"));

	for(int i = 0;i<10;i++)
	{
		DrawableNode* ptr = new DrawableNode();
		ptr->Init(&mBox);
		ptr->mHealth= 100;
		ptr->mPos.x=randf(-5,+5);
		ptr->mPos.y=1;
		ptr->mPos.z=randf(-5,+5);
		mBoxes.push_back(ptr);
	}
	for(int i = 0;i<10;i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mBullets.push_back(ptr);
	}
	mNodeCamera = false;
	//mAxis.mScale=3;
	//mAxis.mHpr.x=XMConvertToRadians(45);
}


void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),
                               D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(),
                               Colors::SkyBlue);
	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology(
                            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// setup the matrixes
	Matrix world;
	Matrix view=mCamera.GetViewMatrix();
	Matrix proj=mCamera.GetDefaultProjectionMatrix();
	// the player:
	mPlayer.Draw(GetContext(),view,proj);
	// the terrain
	mTerrainMat.FillMatrixes(world,view,proj);
	mTerrainMat.Apply(GetContext());
	mpTerrain->Draw(GetContext());
	mpTerrain->Draw(GetContext());



	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);

}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	// keyboard to change render state:
	if (Input::KeyPress(VK_F1))
		GetContext()->RSSetState(
                   mpShaderManager->CommonStates()->CullCounterClockwise());
	if (Input::KeyPress(VK_F2))
		GetContext()->RSSetState(
                   mpShaderManager->CommonStates()->Wireframe());
	if(Input::KeyPress(VK_F3))
	{
		mTerrainMat.mUseTextureBlend = !mTerrainMat.mUseTextureBlend;
	}
	// simple move player
	const float MOVE_SPEED=5;
	float FALL_SPEED = 10;
	Vector3 move=GetKeyboardMovement(KBMOVE_WSAD);
	if (Input::KeyDown(VK_SHIFT))	move*=10;
	mPlayer.Move(move*(MOVE_SPEED*Timer::GetDeltaTime()));
	float h  = mpTerrain->GetHeight(mPlayer.mPos.x,mPlayer.mPos.z);
	
	mPlayer.mPos.y = h +0.7;
	mCamera.SetFocus(mPlayer.GetPos());
	mCamera.Update();

}

void MyApp::UpdateCamera()
{
	//mCamera.LookAt(mPlayer);
	/*
	Vector3 move = GetKeyboardMovement(KBMOVE_WSADRF);
	Vector3 turn=GetKeyboardMovement(KBMOVE_CURSOR_HRP);
	//mPlayer.mPos+=move*(MOVE_SPEED*Timer::GetDeltaTime());
	mCamera.mHpr+=turn*(XMConvertToRadians(90)*Timer::GetDeltaTime());
	mCamera.Move(move*(5*Timer::GetDeltaTime()));

	POINT mouseMove = Input::GetMouseDelta();
	
	mCamera.mHpr.x += mouseMove.x *0.01;
	mCamera.mHpr.y += mouseMove.y *0.01;
	Input::SetMousePos(320,240,GetWindow()); 

	*/
	/*
	mCamera.SetPos(mPlayer.GetPos()+mPlayer.RotateVector(Vector3(0.4f,0.9f,-1.3f)));
	//mCamera.LookAt(mPlayer.GetPos()+mPlayer.RotateVector(Vector3(1.5,0,0)));
	mCamera.mHpr = mPlayer.mHpr;
	*/
	/*
	Vector3 tgt = mPlayer.GetPos()+mPlayer.RotateVector(Vector3(0,2,-5.0f));
	tgt = Vector3::Lerp(mCamera.GetPos(),tgt,5.0f*Timer::GetDeltaTime());

	mCamera.SetPos(tgt);
	mCamera.LookAt(mPlayer);
	*/
/*
	if(mNodeCamera){
		const float SPEED = 3.0f;
		Vector3 move(0,0,0);
		if(Input::KeyDown('A')) move.x--;
		if(Input::KeyDown('D')) move.x++;
	
		if(Input::KeyDown('W')) move.z++;
		if(Input::KeyDown('S')) move.z--;
		mCamera.mPos+=move*SPEED*Timer::GetDeltaTime();
	}*/
}

void MyApp::Shutdown()
{
	DeleteAllNodes(mBoxes);
	DeleteAllNodes(mBullets);
}

// in console C++ is was main()
// in Windows C++ its called WinMain()  (or sometimes wWinMain)
int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine,
	int nShowCmd)
{
	ENABLE_LEAK_DETECTION();
	MyApp app;
	return app.Go(hInstance);	// go!
}



