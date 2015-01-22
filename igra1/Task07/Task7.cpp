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
	
	CameraNode mCamera;
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
	mTitle=L"Task 7a: Node Management";

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
	mPlayer.Init(&mTeapot,Vector3(0,1,0));
	mWatcher.Init(&mTeapot,Vector3(0,2,1));
	mCarShip.Init(&mCarShipObj,Vector3(2,2,2));
	mCarShip.mScale = 0.01;
	

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
	mCamera.Init(Vector3(0,1,-10));
	mCamera.LookAt(Vector3(0,0,0));
	mCamera.SetNearFar(0.05f,100.0f);
	mCamera.mPos= Vector3(0,30,0);
	mCamera.mHpr=Vector3(0,XMConvertToRadians(90),0);
	//mAxis.mScale=3;
	//mAxis.mHpr.x=XMConvertToRadians(45);
}


void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),
                           D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(),
                           DirectX::Colors::SkyBlue);

	// Set Primitive Topology
	GetContext()->IASetPrimitiveTopology(
                             D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ShaderManager::SetSampler(GetContext(),
		mpShaderManager->CommonStates()->AnisotropicWrap());
	// setup the matrixes
	Matrix world;
	Matrix view=XMMatrixLookAtLH(Vector3(0,3,-10), Vector3(0,0,0),
                                    Vector3(0,1,0));
	Matrix proj=XMMatrixPerspectiveFovLH(XM_PI/4,GetAspectRatio(),1,1000);

	if(mNodeCamera)
	{
		view = mCamera.GetViewMatrix();
		proj = mCamera.GetProjectionMatrix();
	}
	/*
	// teapot
	world=Matrix::CreateTranslation(2,1,0);
	mTeapot.mMaterial.FillMatrixes(world,view,proj);
	mTeapot.mMaterial.Apply(GetContext());
	mTeapot.Draw(GetContext());

	world = Matrix::CreateTranslation(0,0,0);
	mPlane.mMaterial.FillMatrixes(world,view,proj);
	mPlane.mMaterial.Apply(GetContext());
	mPlane.Draw(GetContext());

	Draw3DPrepare(GetContext(),mpShaderManager.get(),view,proj);

	*/

	mGround.Draw(GetContext(),view,proj);
	mAxis.Draw(GetContext(),view,proj);
	mPlayer.Draw(GetContext(),view,proj);
	mWatcher.Draw(GetContext(),view, proj);
	mCarShip.Draw(GetContext(),view,proj);
	DrawAliveNodes(mBoxes,GetContext(),view,proj);
	DrawAliveNodes(mBullets,GetContext(),view,proj);


	if(mDiagnostics)
	{
		Draw3DPrepare(GetContext(),mpShaderManager.get(),view,proj);
		mpDraw3D->Begin();
		DrawAliveNodeBounds(mBoxes,mpDraw3D.get(),Colors::Teal);
		DrawAliveNodeBounds(mBullets,mpDraw3D.get(),Colors::Green);
		Draw3DBoundingSphere(mpDraw3D.get(),mPlayer.GetBounds(),Colors::White);
		mpDraw3D->End();
	}
	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);
}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	
	if(Input::KeyPress(VK_SPACE))
	{
		FireShot();
	}

	if(Input::KeyPress(VK_TAB))
	{
		mDiagnostics=!mDiagnostics;
	}

	if(Input::KeyPress(VK_F1))
	{
		mNodeCamera = !mNodeCamera;
	}

	const float MOVE_SPEED = 5,TURN_SPEED=XMConvertToRadians(90),SPIN_SPEED = XMConvertToRadians(60);
	for(unsigned i = 0;i<mBoxes.size();i++)
	{
		mBoxes[i]->Yaw(SPIN_SPEED*Timer::GetDeltaTime());
	}
	UpdateAliveNodes(mBullets);
	CheckCollisions();
	mWatcher.LookAt(mPlayer);
	
	Vector3 move = GetKeyboardMovement(KBMOVE_WSADRF);
	Vector3 turn=GetKeyboardMovement(KBMOVE_CURSOR_HRP);
	//mPlayer.mPos+=move*(MOVE_SPEED*Timer::GetDeltaTime());
	//mPlayer.mHpr+=turn*(TURN_SPEED*Timer::GetDeltaTime());
	mPlayer.Turn(turn*(TURN_SPEED*Timer::GetDeltaTime()));
	mPlayer.Move(move*(MOVE_SPEED*Timer::GetDeltaTime()));
	UpdateCamera();
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

	if(mNodeCamera){
		const float SPEED = 3.0f;
		Vector3 move(0,0,0);
		if(Input::KeyDown('A')) move.x--;
		if(Input::KeyDown('D')) move.x++;
	
		if(Input::KeyDown('W')) move.z++;
		if(Input::KeyDown('S')) move.z--;
		mCamera.mPos+=move*SPEED*Timer::GetDeltaTime();
	}
}

void MyApp::Shutdown()
{
	DeleteAllNodes(mBoxes);
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



