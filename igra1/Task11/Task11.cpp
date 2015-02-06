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
#include "Node.h"

#include "Draw3D.h"
#include "Terrain.h"

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
	void FireShot();
	void FireCluster();
	void CheckCollisions();	

	Vector2 mCentreOfScreen;

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;
	std::unique_ptr<PrimitiveBatch<ColouredVertex>> mpDraw3D;

	CComPtr<ID3D11ShaderResourceView> mpCubemap;
	ModelObj mRobot,mPlane,mSkyObj,mBarracksObj,mYellowBallObj;
	void DrawShadows(const Matrix& view,const Matrix& proj) ;
	BasicMaterial mShadowMaterial;
	CComPtr<ID3D11DepthStencilState> mpShadowStencil;
	std::vector<Bullet*> mBullets;
	std::vector<Bullet*> mClusters;
	
	//ArcBallCamera mCamera;
	CameraNode mCamera;
	Vector3 mCurrentCamMode, mCurrentCamAngle;
	Vector3 mTPScam, mFPScam, mShoulderCam, mBackCam;
	Vector3 mTPSangle, mFPSangle, mShoulderAngle, mBackAngle;

	DrawableNode mPlayer;
	float maxHeight;
	
	//Temp Variables
	float mHealth;
	float mEnergy;
	int mScore;
	float mRecoverDelay;
	float mEnergyBlink, mAlphaModifier;

	float mClusterActive, mClusterTimer;

	std::unique_ptr<Terrain> mpTerrain;
	TerrainMaterial mTerrainMat;

	std::unique_ptr<SpriteBatch> mpSpriteBatch;
	std::unique_ptr<SpriteFont> mpSpriteFont;
 	CComPtr<ID3D11ShaderResourceView> mpTexLifeBar;

	std::wstring testStr;
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
	
	mEnergy--;
	ptr->mHealth=100;
	ptr->mLifetime= 3.0f;
	ptr->SetPos(mPlayer.GetPos() + mPlayer.RotateVector(Vector3(-0.75f,3.8f,1)));
	ptr->mVelocity = mPlayer.RotateVector(Vector3(3,-3.6f,20));
	ptr->mScale=0.4f;
}
void MyApp::FireCluster()
{
	/*
	Bullet* ptr = FindDeadNode(mBullets);
	if(ptr== nullptr)return;
	*/

	//float rot = 40;
	for(int i = 0; i < mClusters.size(); i++)
	{
		float rot = randf(-40, 40);
		Bullet* ptr = mClusters[i];
		ptr->mHealth=100;
		ptr->mLifetime= 3.0f;
		ptr->SetPos(mPlayer.GetPos() + mPlayer.RotateVector(Vector3(0,3,-0.8f)));
		//ptr->Yaw(mPlayer.GetHpr().z);
		ptr->SetHpr(mPlayer.GetHpr().x, 0, XMConvertToRadians(rot));
		ptr->mVelocity = mClusters[i]->RotateVector(Vector3(0,12,-50 * mClusterTimer));
		ptr->mScale=0.6f;
	}
}
void MyApp::CheckCollisions()
{
	for(int b = 0;b<mBullets.size();b++)
	{
		if(mBullets[b]->IsAlive()==false) continue;
		
		BoundingSphere bs = mBullets[b]->GetBounds();

		/*for(int t = 0;t <mBoxes.size();t++)
		{
			if(mBoxes[t]->IsAlive()==false)continue;

			if(bs.Intersects(mBoxes[t]->GetBounds()))
			{
				mBullets[b]->Kill();
				mBoxes[t]->mHealth-=25;
			}
		}*/
	}
}

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Task 11: BUILDING";

	ShowCursor(false);
	mCentreOfScreen = Vector2(GetWindowRect().right / 2,GetWindowRect().bottom / 2);

	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	
	// initial pos/tgt for camera
	//mCamera.Reset();

//	mpTerrain.reset(new Terrain(GetDevice(),L"../Content/CA Terrain/Game.bmp",
      //                     Vector3(1,0.3f,1)));
	mpTerrain.reset(new Terrain(GetDevice(),L"../Content/CA Terrain/Game2.bmp",
                            Vector3(2,0.3f,2)));
	mTerrainMat.mpShaderManager=mpShaderManager.get();
	mTerrainMat.mpTexBase.Attach(CreateTextureResourceWIC(GetDevice(),
                          L"../Content/CA Terrain/Game_d.png"));

	std::vector<std::wstring> textures;
	textures.push_back(L"../Content/CA Terrain/Seamless_Grass.bmp");
	textures.push_back(L"../Content/CA Terrain/Seamless_Rock.bmp");
	textures.push_back(L"../Content/CA Terrain/Seamless_DirtPath.bmp");
	textures.push_back(L"../Content/CA Terrain/Seamless_DarkRock.bmp");

	mTerrainMat.mpTexLayerArray.Attach(CreateTexture2DArray(GetDevice(),GetContext(),textures));
	mTerrainMat.mpTexLight.Attach(CreateTextureResourceWIC(
                               GetDevice(),L"../Content/CA Terrain/Game_l.png"));
	// load models:
	mRobot.Load(GetDevice(),mpShaderManager.get(),L"../Content/Mech-Mk1.obj",
                         true);
	mBarracksObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/Barracks.obj",
                         true);
	mPlane.Load(GetDevice(),mpShaderManager.get(),L"../Content/plane.obj",true);
	mSkyObj.Load(GetDevice(),mpShaderManager.get(),
                               L"../Content/skyball.obj",false);
	mYellowBallObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/yellow_ball.obj");
	mpCubemap.Attach(CreateTextureResourceDDS(GetDevice(),
                               L"../Content/Cubemaps/grasscube1024.dds"));
	
	for(int i = 0;i<20;i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mBullets.push_back(ptr);
	}

	for(int i = 0;i<8;i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mClusters.push_back(ptr);
	}

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

	mRobot.mMaterial.mMaterial.gMaterial.Specular=Color(1,1,1,1);
	mRobot.mMaterial.mMaterial.gMaterial.Specular.w=20;	// power
	mRobot.mMaterial.mMaterial.gMaterial.Reflect=Color(0.1f,0.1f,1,1)*0.5f;
	mRobot.mMaterial.mMaterial.gEnableReflection=true;

	mHealth = 100.0f;
	mEnergy = 100.0f;
	mScore = 0;
	mRecoverDelay = 0;
	mEnergyBlink = 1.0f;

	mClusterActive = false;
	mClusterTimer = 0;

	mpSpriteBatch.reset(new SpriteBatch(GetContext()));
	mpSpriteFont.reset(new SpriteFont(GetDevice(),L"../Content/Times12.sprfont"));
	mpTexLifeBar = CreateTextureResourceWIC(GetDevice(),L"../Content/LifeBar.jpg");

	mPlayer.Init(&mRobot, Vector3(0,0,0));
	mCamera.Init(Vector3(0,0,0));
	
	maxHeight = mpTerrain->GetHeight(mPlayer.mPos.x, mPlayer.mPos.z) + 2;

	/*mTPScam = Vector3(-2,7,-8);
	mTPSangle = Vector3(0,3,4);*/
	mTPScam = Vector3(-2,7,-15);
	mTPSangle = Vector3(0,3,4);

	mFPScam = Vector3(0,4.5f,1);
	mFPSangle = Vector3(0,3,5);

	mShoulderCam = Vector3(2,5,-2);
	mShoulderAngle = Vector3(0,4,6);

	mBackCam = Vector3(2,6,3);
	mBackAngle = Vector3(0,2,-6);

	mCurrentCamMode = mTPScam;
	mCurrentCamAngle = mTPSangle;
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

	const float BLEND_FACTOR[] = {0.0f, 0.0f, 0.0f, 0.0f};
	const unsigned BLEND_MASK=0xffffffff;
	GetContext()->OMSetDepthStencilState(mpShadowStencil,0);

	// setup the matrixes
	Matrix world;
	Matrix view=mCamera.GetViewMatrix();
	Matrix proj=mCamera.GetProjectionMatrix();

	// the models:
	world=Matrix::CreateTranslation(0,0,0);
	mPlane.mMaterial.FillMatrixes(world,view,proj);
	mPlane.mMaterial.Apply(GetContext());
	mPlane.Draw(GetContext());
	
	GetContext()->OMSetDepthStencilState(nullptr,0);

	world= Matrix::CreateTranslation(0,0,0);
	mTerrainMat.FillMatrixes(world,view,proj);
	mTerrainMat.Apply(GetContext());
	mpTerrain->Draw(GetContext());
	//mpTerrain->Draw(GetContext());

	

	//world=Matrix::CreateTranslation(0,0,0) * Matrix::CreateScale(0.1f);
	//mBarracksObj.mMaterial.FillMatrixes(world,view,proj);
	//mBarracksObj.mMaterial.mLights.gEyePosW=mCamera.GetCamPos();
	//mBarracksObj.mMaterial.Apply(GetContext());
	//mBarracksObj.Draw(GetContext());

	/*world=Matrix::CreateTranslation(mPlayerPos.x,mPlayerPos.y,mPlayerPos.z) * Matrix::CreateScale(0.1f);
	mPlayer.mMaterial.FillMatrixes(world,view,proj);
	mPlayer.mMaterial.mLights.gEyePosW=mCamera.GetPos();
	mPlayer.mMaterial.Apply(GetContext());*/
	// EXTRA: add the cubemap as T1
	ID3D11ShaderResourceView* tex[]={mpCubemap};
	GetContext()->PSSetShaderResources(1,1,tex);	// set as index T1
	mPlayer.Draw(GetContext(),view,proj);
	mPlayer.mScale = 0.1f;
	//DrawShadows(view,proj);
	// draw sky
	world= Matrix::CreateTranslation(mCamera.GetPos());
	mSkyObj.mMaterial.FillMatrixes(world,view,proj);
	mSkyObj.mMaterial.Apply(GetContext());
	mSkyObj.Draw(GetContext());
	// Present the backbuffer to the screen

	DrawAliveNodes(mBullets,GetContext(),view,proj);
	DrawAliveNodes(mClusters,GetContext(),view,proj);
	
	GetContext()->OMSetDepthStencilState(
                            mpShaderManager->CommonStates()->DepthDefault(),0);
	//XMMATRIX matrix = XMMATRIX(mEnergy/10,0,0,0,0,1,0,0,0,0,1,0,-1,1,0,1);

	RECT rec;
	rec.left = 50;
	rec.top = 50;
	rec.right = 400;
	rec.bottom = 75;

	const RECT* rect = &rec;
	
	mpSpriteBatch->Begin();
	mpSpriteBatch->Draw(mpTexLifeBar,XMFLOAT2(mSize.right/12-40,mSize.bottom/12-30),rect,DirectX::Colors::White * mEnergyBlink,0.0f,XMFLOAT2(0,0),XMFLOAT2((mEnergy/50),1),DirectX::SpriteEffects::SpriteEffects_None,0.0f);
	std::wstring scoreStr = ToString("Score: "+mScore);
	mpSpriteFont->DrawString(mpSpriteBatch.get(),scoreStr.c_str(),Vector2(GetWindowRect().right - 201, 39),Colors::Black);
	mpSpriteFont->DrawString(mpSpriteBatch.get(),scoreStr.c_str(),Vector2(GetWindowRect().right - 200, 40),Colors::White);
	mpSpriteFont->DrawString(mpSpriteBatch.get(),testStr.c_str(),Vector2(199,199), Colors::Black);
	mpSpriteFont->DrawString(mpSpriteBatch.get(),testStr.c_str(),Vector2(200,200), Colors::White);
	mpSpriteBatch->End();

	GetContext()->OMSetBlendState(mpShaderManager.get()->CommonStates()->Opaque(),BLEND_FACTOR,BLEND_MASK);
	GetContext()->OMSetDepthStencilState(
                            mpShaderManager->CommonStates()->DepthDefault(),0);
	//GetContext()->OMSetDepthStencilState(nullptr,0);

	GetSwapChain()->Present(0, 0);

}
void MyApp::DrawShadows(const Matrix& view,const Matrix& proj)
{
	//Matrix world;
	//// create shadow matrix:
	//Plane ground(Vector3(0,1,0),0); // ground's normal is up & pos is 0
	//// getting the strongest light source (gDirLights[0]) to make the shadow
	//Vector3 toLight=-mPlayer.mMaterial.mLights.gDirLights[0].Direction;
	//// make matrix (see Luna for the math details)
	//Matrix shadowMatrix=Matrix::CreateShadow(toLight,ground) * Matrix::CreateScale(0.1f);
	//// add a very small offset to keep it out of the ground
	//shadowMatrix*=Matrix::CreateTranslation(0,0.01f,0);

	//const float BLEND_FACTOR[] = {0.0f, 0.0f, 0.0f, 0.0f};
	//const unsigned BLEND_MASK=0xffffffff;
	//GetContext()->OMSetBlendState(
 //           mpShaderManager->CommonStates()->NonPremultiplied(),
 //           BLEND_FACTOR,BLEND_MASK);
	//GetContext()->OMSetDepthStencilState(mpShadowStencil,0);

	//// draw objects almost as normal:
	//// just using a different material & a different world matrix

	//GetContext()->OMSetDepthStencilState(mpShadowStencil,1);

	//world=Matrix::CreateTranslation(0,1,0) * shadowMatrix;
	//mShadowMaterial.FillMatrixes(world,view,proj);
	//mShadowMaterial.Apply(GetContext());
	//mPlayer.Draw(GetContext());
	//// draw all the rest using the world*shadowMatrix

	//// turn off blend 
	//GetContext()->OMSetBlendState(
 //              mpShaderManager->CommonStates()->Opaque(),
 //              BLEND_FACTOR,BLEND_MASK);
	//GetContext()->OMSetDepthStencilState(nullptr,0);
}

void MyApp::Update()
{
	//SetCursorPos(mCentreOfScreen.x, mCentreOfScreen.y);

	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();

#pragma region Player Shooting Functions & Effects
	if(mEnergy<=100.0f){
		if(Input::KeyDown(VK_UP))
			mEnergy++;
	}
	if(mEnergy>=0)
	{
		if(Input::KeyDown(VK_DOWN))
			mEnergy--;
	}
	if(Input::KeyPress(VK_LBUTTON))
	{
		if (mEnergy>=0)
		{	
			mRecoverDelay = 0;
			FireShot();
		}
	}
	else
	{
		// Recovery after shooting
		if(mRecoverDelay <= 1)
		{
			mRecoverDelay += Timer::GetDeltaTime();
		}
		else if(mEnergy <= 100.0f)
		{
			mEnergy += 4 * Timer::GetDeltaTime();
		}
	}

	if(mClusterActive == false)
	{
		if(Input::KeyPress(0x5A))
		{
			FireCluster();
			mClusterActive = true;
		}	
	}

	if(mClusterActive == true)
	{
		mClusterTimer += Timer::GetDeltaTime();

		if(mClusterTimer <= 0.5f)
		{
			for(int i = 0; i < mClusters.size(); i++)
			{
				mClusters[i]->mVelocity = mClusters[i]->RotateVector(Vector3(0, 12, -50 + (200 * mClusterTimer)));
			}
		}
		else
		{
			for(int i = 0; i < mClusters.size(); i++)
			{
				float randX = randf(-5, 5);
				float randY = randf(-10, 0);
				//mClusters[i]->SetHpr(0, XMConvertToRadians(90), 0);
				//mClusters[i]->LookAt(Vector3(0, 0, randZ));
				mClusters[i]->mVelocity = mClusters[i]->RotateVector(Vector3(randX, randY, 20));
			}
		}

		if(mClusterTimer >= 3.0f)
		{
			mClusterActive = false;
			mClusterTimer = 0;
		}
	}

	// Blinking effect when energy is low
	if(mEnergy <= 20.0f)
	{
		if(mEnergyBlink <= 0)
			mAlphaModifier = 2 * Timer::GetDeltaTime();
		else if(mEnergyBlink >= 1)
			mAlphaModifier = -2 * Timer::GetDeltaTime();

		mEnergyBlink += mAlphaModifier;
	}
	else
	{
		mEnergyBlink = 1.0f;
	}
#pragma endregion

#pragma region Camera Modes Switch
	if (Input::KeyPress(VK_F1))
	{
		mCurrentCamMode = mTPScam;
		mCurrentCamAngle = mTPSangle;
	}
	if (Input::KeyPress(VK_F2))
	{
		mCurrentCamMode = mFPScam;
		mCurrentCamAngle = mFPSangle;
	}
	if (Input::KeyPress(VK_F3))
	{
		mCurrentCamMode = mShoulderCam;
		mCurrentCamAngle = mShoulderAngle;
	}
	if(Input::KeyPress(VK_F4))
	{
		mCurrentCamMode = mBackCam;
		mCurrentCamAngle = mBackAngle;
	}
#pragma endregion
	
	UpdateAliveNodes(mBullets);
	UpdateAliveNodes(mClusters);
	//CheckCollisions();

	Input::SetMousePos(mCentreOfScreen.x, mCentreOfScreen.y, GetWindow());

	float MOVE_SPEED = 5;
	float TURN_SPEED = XMConvertToRadians(60);
	Vector3 move = GetKeyboardMovement(KBMOVE_WSAD);
	Vector3 turn = GetMouseTurn();
	turn.y = 0;

	mPlayer.Move(move*MOVE_SPEED*Timer::GetDeltaTime());
	mPlayer.mPos.y = mpTerrain->GetHeight(mPlayer.mPos.x, mPlayer.mPos.z);

	if(mPlayer.mPos.y > maxHeight)
		mPlayer.Move(-move*MOVE_SPEED*Timer::GetDeltaTime());

	mPlayer.Turn(turn*TURN_SPEED*Timer::GetDeltaTime());
	testStr = ToString("Cluster Timer: ", mClusterTimer);

	mCamera.SetPos(mPlayer.GetPos() + mPlayer.RotateVector(mCurrentCamMode));
	mCamera.LookAt(mPlayer.GetPos() + mPlayer.RotateVector(mCurrentCamAngle));
	//mCamera.LookAt(Vector3(0,0,3));
	//mCamera.Update();
}
void MyApp::Shutdown()
{
	DeleteAllNodes(mBullets);
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



