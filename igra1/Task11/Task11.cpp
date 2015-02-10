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

#include <mmsystem.h>
#include <stdio.h>
#include <irrKlang.h>

using namespace irrklang;

#pragma comment(lib, "irrKlang.lib")

#if defined(WIN32)
#include <conio.h>
#else
#include "../common/conio.h"
#endif

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

class Enemy : public DrawableNode
{
public:
	Enemy(std::vector<Vector3>);
	//~Enemy();
	Vector3 mVelocity,mDirection,mPrevPos;
	Vector3* mEnemyPos;
	BOOL attackBool;
	BOOL mTooNearBool;
	float shotTimer;
	void Update();
	void Activate();
};

Enemy::Enemy(std::vector<Vector3> mPosition)
{
	mVelocity = Vector3(0,0,0);
	mDirection = Vector3(0,0,0);
	int i = rand() % (int)(4 - 0 + 1);
	mPos = mPosition[i];
	mHealth = 100.0f;
	attackBool = false;
};

class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	void Shutdown();
	void FireShot();
	void FireEnemyShot(Enemy*);
	void FireCluster();
	void DisplayClusterAoE();
	void PlayerCollision();
	void CheckPlayerCollisions(); // Check whether player get shot
	void CheckEnemyCollisions(); // check whether enemy get shot
	void DeployEnemy(int);
	void SetupBarracks();
	void SetupTrees();
	void Restart();
	float GetHeight(Vector3);

	Vector2 mCentreOfScreen;

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;
	std::unique_ptr<PrimitiveBatch<ColouredVertex>> mpDraw3D;

	CComPtr<ID3D11ShaderResourceView> mpCubemap;
	ModelObj mRobot,mSkyObj,mBarracksObj,mYellowBallObj,mEnemyObj, mTreeObj;
	void DrawShadows(const Matrix& view,const Matrix& proj);
	BasicMaterial mShadowMaterial;
	CComPtr<ID3D11DepthStencilState> mpShadowStencil;
	std::vector<Bullet*> mBullets;
	std::vector<Bullet*> mEnemyBullets;
	std::vector<Bullet*> mClusters, mClusterAoE;
	std::vector<Enemy*> mEnemies;
	std::vector<DrawableNode*> mBarracks, mTrees;
	
	//ArcBallCamera mCamera;
	CameraNode mCamera;
	Vector3 mCurrentCamMode, mCurrentCamAngle;
	Vector3 mTPScam, mFPScam, mShoulderCam, mBackCam;
	Vector3 mTPSangle, mFPSangle, mShoulderAngle, mBackAngle;
	
	DrawableNode mPlayer;
	float maxHeight;
	
	irrklang::ISoundEngine* engine;
	irrklang::ISoundEngine* sfxEngine;
	float mVolume;

	//Temp Variables
	float mHealth;
	float mEnergy;
	int mScore,mhighScore;
	float mRecoverDelay;
	float mSpawnDelay;
	float mEnergyBlink, mAlphaModifier;

	float mPlayerDamage;
	float mAIDamage;

	bool isBoom;
	float mClusterActive, mClusterTimer, mClusterArmed;

	float mTextBlinker;

	TerrainMaterial mTerrainMat;

	std::unique_ptr<SpriteBatch> mpSpriteBatch;

	std::unique_ptr<SpriteFont> mpSpriteFont,mpVikingFont72;
 	CComPtr<ID3D11ShaderResourceView> mpTexLifeBar,mpGameOverScreen,mpGameOverText,mpEsc2Quit, mpGameWinScreen;

	std::vector<Vector3> mSpawnLocations;

	std::wstring strClusterCD, strCluster;

	bool mEndGameCond;

	public :
		std::unique_ptr<Terrain> mpTerrain;

		float mDiagnostics;
};

void MyApp::Startup()
{
	// set the title (needs the L"..." because of Unicode)
	mTitle=L"Mech-allion";

	ShowCursor(false);
	mCentreOfScreen = Vector2(GetWindowRect().right / 2,GetWindowRect().bottom / 2);
	
	mpDraw3D.reset(new PrimitiveBatch<ColouredVertex>(GetContext()));

	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));
	
	engine = irrklang::createIrrKlangDevice();
	sfxEngine = irrklang::createIrrKlangDevice();

	ISound* music = engine->play2D("../Content/Sounds/Tank.mp3",true);
	mVolume = 0.1f;
	engine->setSoundVolume(mVolume);
	sfxEngine->setSoundVolume(0.2f);

	

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
	mSkyObj.Load(GetDevice(),mpShaderManager.get(),
                               L"../Content/skyball.obj",false);
	mYellowBallObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/yellow_ball.obj");
	mEnemyObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/Cube-Soldier.obj");
	mTreeObj.Load(GetDevice(),mpShaderManager.get(),L"../Content/Cartoon_Pine_Tree.obj");
	mpCubemap.Attach(CreateTextureResourceDDS(GetDevice(),
                               L"../Content/Cubemaps/grasscube1024.dds"));
	
	mSkyObj.mMaterial.mpVS=mpShaderManager->VSSkybox();
	mSkyObj.mMaterial.mpLayout=mpShaderManager->LayoutSkybox();
	mSkyObj.mMaterial.mpPS=mpShaderManager->PSSkybox();
	mSkyObj.mMaterial.mpTexture=mpCubemap;
	mSkyObj.mMaterial.mMaterial.gUseTexture=true;
	
#pragma region Vectors
	// Player's bullets
	for(int i = 0;i<20;i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mBullets.push_back(ptr);
	}

	// Enemies' bullets
	for (int i = 0; i < 50; i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mEnemyBullets.push_back(ptr);
	}

	// Cluster "Missile"
	for(int i = 0;i<8;i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mClusters.push_back(ptr);
	}

	// Cluster AoE Circle
	for(int i = 0; i < 12; i++)
	{
		Bullet* ptr = new Bullet();
		ptr->Init(&mYellowBallObj);
		ptr->Kill();
		mClusterAoE.push_back(ptr);
	}

	// Tree
	for(int i = 0; i < 30; i++)
	{
		DrawableNode* ptr = new DrawableNode();
		ptr->Init(&mTreeObj);
		ptr->Kill();
		mTrees.push_back(ptr);
	}
#pragma endregion
	
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
	

	mRobot.mMaterial.mMaterial.gMaterial.Specular=Color(1,1,1,1);
	mRobot.mMaterial.mMaterial.gMaterial.Specular.w=20;	// power
	mRobot.mMaterial.mMaterial.gMaterial.Reflect=Color(0.1f,0.1f,1,1)*0.5f;
	mRobot.mMaterial.mMaterial.gEnableReflection=true;

	mSpawnLocations.push_back(Vector3(-100,mpTerrain->GetHeight(-100,10),10));
	mSpawnLocations.push_back(Vector3(-10,mpTerrain->GetHeight(-10,-100),-100));
	mSpawnLocations.push_back(Vector3(85,mpTerrain->GetHeight(85,-30),-30));
	mSpawnLocations.push_back(Vector3(90,mpTerrain->GetHeight(90,90),90));
	mSpawnLocations.push_back(Vector3(-60,mpTerrain->GetHeight(-60,60),60));

	for(unsigned i = 0;i < 10;i++)
	{
		Enemy* ptr = new Enemy(mSpawnLocations);
		ptr->Init(&mEnemyObj);
		ptr->Kill();
		mEnemies.push_back(ptr);
	}

	for(unsigned i = 0; i < mSpawnLocations.size(); i++)
	{
		DrawableNode* ptr = new DrawableNode();
		ptr->Init(&mBarracksObj);
		ptr->Kill();
		mBarracks.push_back(ptr);
	}

	mSpawnDelay = 0.0f;

	mHealth = 100.0f;
	mEnergy = 100.0f;
	mScore = 0;
	mRecoverDelay = 0;
	mEnergyBlink = 1.0f;
	mTextBlinker = 1.0f;

	mClusterActive = false;
	mClusterArmed = false;
	mClusterTimer = 0;

	mPlayerDamage = 25;
	mAIDamage = 2;

	mDiagnostics = false;

	mpSpriteBatch.reset(new SpriteBatch(GetContext()));

	mpSpriteFont.reset(new SpriteFont(GetDevice(),L"../Content/Viking20.sprfont"));
	mpVikingFont72.reset(new SpriteFont(GetDevice(),L"../Content/Viking72.sprfont"));

	mpTexLifeBar = CreateTextureResourceWIC(GetDevice(),L"../Content/LifeBar.jpg");
	mpGameOverScreen = CreateTextureResourceWIC(GetDevice(),L"../Content/Game-Over.jpg");
	mpGameOverText = CreateTextureResourceWIC(GetDevice(),L"../Content/GameOver_Text.jpg");
	mpEsc2Quit = CreateTextureResourceWIC(GetDevice(),L"../Content/Esc2Quit.jpg");
	mpGameWinScreen = CreateTextureResourceWIC(GetDevice(),L"../Content/Win-Screen.jpg");

	mPlayer.Init(&mRobot, Vector3(-80,0,-100));
	mCamera.Init(Vector3(0,0,0));
	
	maxHeight = mpTerrain->GetHeight(mPlayer.mPos.x, mPlayer.mPos.z) + 10;

	/*mTPScam = Vector3(-2,7,-8);
	mTPSangle = Vector3(0,3,4);*/
	mTPScam = Vector3(-2,6,-8);
	mTPSangle = Vector3(0,3,4);

	mFPScam = Vector3(0,3,1);
	mFPSangle = Vector3(0,2,4);

	mShoulderCam = Vector3(1.5f,2,-1.5f);
	mShoulderAngle = Vector3(1,2,5);

	mBackCam = Vector3(2,4,5);
	mBackAngle = Vector3(0,2,-6);

	mCurrentCamMode = mTPScam;
	mCurrentCamAngle = mTPSangle;

	SetupBarracks();
	SetupTrees();

	mEndGameCond = false;
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
	
	if(mEnergy>0.0f)
	{
		world= Matrix::CreateTranslation(mCamera.GetPos());
		mSkyObj.mMaterial.FillMatrixes(world,view,proj);
		mSkyObj.mMaterial.Apply(GetContext());
		mSkyObj.Draw(GetContext());
		// the models:
	
		GetContext()->OMSetDepthStencilState(nullptr,0);

		world= Matrix::CreateTranslation(0,0,0);
		mTerrainMat.FillMatrixes(world,view,proj);
		mTerrainMat.Apply(GetContext());
		mpTerrain->Draw(GetContext());
		//mpTerrain->Draw(GetContext());

		//for(unsigned i = 0;i<mSpawnLocations.size();i++)
		//{
			/*world= Matrix::CreateScale(0.2f) * Matrix::CreateTranslation(mSpawnLocations[i] + Vector3(0,2,0));
			mBarracksObj.mMaterial.FillMatrixes(world,view,proj);
			mBarracksObj.mMaterial.Apply(GetContext());
			mBarracksObj.Draw(GetContext());*/
		//}
	
	
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
		mPlayer.mHealth = 1000;
		mPlayer.mScale = 0.1f;
		//DrawShadows(view,proj);
		// draw sky
		// Present the backbuffer to the screen

		DrawAliveNodes(mBullets,GetContext(),view,proj);
		DrawAliveNodes(mEnemyBullets,GetContext(),view,proj);
		DrawAliveNodes(mClusters,GetContext(),view,proj);
		DrawAliveNodes(mClusterAoE,GetContext(),view,proj);
		DrawAliveNodes(mEnemies,GetContext(),view,proj);
		DrawAliveNodes(mBarracks,GetContext(),view,proj);
		DrawAliveNodes(mTrees,GetContext(),view,proj);
	
		if(mDiagnostics)
		{
			Draw3DPrepare(GetContext(),mpShaderManager.get(),view,proj);
			mpDraw3D->Begin();
			//DrawAliveNodeBounds(mBullets,mpDraw3D.get(),Colors::Green);
			DrawAliveNodeBounds(mEnemies,mpDraw3D.get(),Colors::Red);
			DrawAliveNodeBounds(mTrees,mpDraw3D.get(),Colors::Yellow);
			Draw3DBoundingBox(mpDraw3D.get(),mPlayer.GetBox(),Colors::White);
			mpDraw3D->End();
		}

		GetContext()->OMSetDepthStencilState(
								mpShaderManager->CommonStates()->DepthDefault(),0);
		//XMMATRIX matrix = XMMATRIX(mEnergy/10,0,0,0,0,1,0,0,0,0,1,0,-1,1,0,1);

		RECT rec;
		rec.left = 50;
		rec.top = 50;
		rec.right = 400;
		rec.bottom = 75;

		const RECT* rect = &rec;

		if(mClusterArmed)
		{
			strCluster=L"Cluster: Armed";
		}
		else
		{
			strCluster=L"Cluster: Active";
			strClusterCD=L"";
		}

		if(mClusterActive)
		{
			strCluster=L"Cluster: Recharging";
			strClusterCD = ToString("Cooldown: ", (int)(4 - mClusterTimer));
		}

		mpSpriteBatch->Begin();
		mpSpriteBatch->Draw(mpTexLifeBar,XMFLOAT2(mSize.right/12-40,mSize.bottom/12-30),rect,DirectX::Colors::White * mEnergyBlink,0.0f,XMFLOAT2(0,0),XMFLOAT2((mEnergy/50),1),DirectX::SpriteEffects::SpriteEffects_None,0.0f);
		std::wstring scoreStr = ToString("Score: ", mScore);
		mpSpriteFont->DrawString(mpSpriteBatch.get(),scoreStr.c_str(),Vector2(GetWindowRect().right - 260, 35),Colors::Black);
		mpSpriteFont->DrawString(mpSpriteBatch.get(),scoreStr.c_str(),Vector2(GetWindowRect().right - 261, 37),Colors::White);
		mpSpriteFont->DrawString(mpSpriteBatch.get(),strCluster.c_str(),Vector2(40, 60),Colors::Black);
		mpSpriteFont->DrawString(mpSpriteBatch.get(),strCluster.c_str(),Vector2(41, 62),Colors::White);
		mpSpriteFont->DrawString(mpSpriteBatch.get(),strClusterCD.c_str(),Vector2(40, 80),Colors::Black);
		mpSpriteFont->DrawString(mpSpriteBatch.get(),strClusterCD.c_str(),Vector2(41, 82),Colors::White);
		mpSpriteBatch->End();
	}
	else
	{
		RECT rec;
		rec.left = 0;
		rec.top = 0;
		rec.bottom = 768;
		rec.right = 1024;

		std::wstring mFinalScore = ToString("Final Score: ", mScore);

		const RECT* rec1 = &rec;

		mpSpriteBatch->Begin();
		if(mEndGameCond)
		{
			mpSpriteBatch->Draw(mpGameWinScreen,XMFLOAT2(0,0),rec1,DirectX::Colors::White,0.0f,XMFLOAT2(0,0),XMFLOAT2(1,1),DirectX::SpriteEffects::SpriteEffects_None,0.0f);
		}
		else
		{
			mpSpriteBatch->Draw(mpGameOverScreen,XMFLOAT2(0,0),rec1,DirectX::Colors::White,0.0f,XMFLOAT2(0,0),XMFLOAT2(1,1),DirectX::SpriteEffects::SpriteEffects_None,0.0f);
		}

		rec.bottom = 70;
		rec.right = 700;

		const RECT* rec2 = &rec;
		float flash=((sinf(Timer::GetTime()*2)+1)/2)+0.4f;

		mpSpriteBatch->Draw(mpGameOverText,XMFLOAT2(mSize.right/2,(mSize.bottom/3)*2-50),rec2,DirectX::Colors::White*flash,0.0f,XMFLOAT2(350,35),XMFLOAT2(1,1),DirectX::SpriteEffects::SpriteEffects_None,0.0f);

		rec.right = 400;
		rec.bottom = 70;

		mpSpriteBatch->Draw(mpEsc2Quit,XMFLOAT2(mSize.right/2,(mSize.bottom/3)*2+20),rec2,DirectX::Colors::White,0.0f,XMFLOAT2(200,35),XMFLOAT2(1,1),DirectX::SpriteEffects::SpriteEffects_None,0.0f);
		
		mpVikingFont72->DrawString(mpSpriteBatch.get(),mFinalScore.c_str(),Vector2(180,320),Colors::White);
		mpVikingFont72->DrawString(mpSpriteBatch.get(),mFinalScore.c_str(),Vector2(181,322),Colors::Red);

		mpSpriteBatch->End();

		mTextBlinker = ((sin(mTextBlinker)+1)/2);

		/*if(mTextBlinker>=0.3f)
		{
			if(mTextBlinker<=1.0f)
			{
				mTextBlinker += 0.5f*Timer::GetDeltaTime();
			}
			else 
			{
				mTextBlinker = 0.3f;
			}
		}
		else if(mTextBlinker<=1.0f)
		{
			if(mTextBlinker>=0.3f)
			{
				mTextBlinker -= 0.5f*Timer::GetDeltaTime();
			}
			else 
			{
				mTextBlinker = 0.3f;
			}
		}
		else 
		{
			mTextBlinker = 0.3f;
		}*/

	}

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

	/*if(Input::KeyPress(VK_TAB))
		mDiagnostics = !mDiagnostics;*/

	if(mEnergy>0.0f){
	#pragma region Player Shooting Functions & Effects
		/*if(mEnergy<=100.0f){
			if(Input::KeyDown(VK_UP))
				mEnergy++;
		}
		if(mEnergy>=0)
		{
			if(Input::KeyDown(VK_DOWN))
				mEnergy--;
		}*/
		if(Input::KeyPress(VK_LBUTTON))
		{
			// Shoot regular bullet
			if(!mClusterArmed)
			{
				if (mEnergy>=0)
				{	
					mRecoverDelay = 0;
					FireShot();
				}
			}
			else
			{
				for(int i = 0; i < mClusterAoE.size(); i++)
				{
					mClusterAoE[i]->Kill();
				}

				FireCluster();
				mClusterActive = true;
				mClusterArmed = false;
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

		isBoom = false;

		if(mClusterActive == false)
		{
			if(Input::KeyPress(0x5A))
			{
				mClusterArmed = !mClusterArmed;

				if(!mClusterArmed)
				{
					for(int i = 0; i < mClusterAoE.size(); i++)
					{
						mClusterAoE[i]->Kill();
					}
				}
			}	
		}
	
		if(mClusterArmed)
		{
			DisplayClusterAoE();
		}

		if(mClusterActive == true)
		{
			mClusterTimer += Timer::GetDeltaTime();

			if(mClusterTimer <= 0.5f)
			{
				for(int i = 0; i < mClusters.size(); i++)
				{
					mClusters[i]->mVelocity = mClusters[i]->RotateVector(Vector3(0, 5, -30 + (150 * mClusterTimer)));
				}
			}
			else
			{
				for(int i = 0; i < mClusters.size(); i++)
				{
					float randX = randf(-40, 40);
					float randY = randf(-20, -15);
					//mClusters[i]->SetHpr(0, XMConvertToRadians(90), 0);
					//mClusters[i]->LookAt(Vector3(0, 0, randZ));
					//mClusters[i]->mVelocity = mClusters[i]->RotateVector(Vector3(randX, randY, 20));
					mClusters[i]->SetHpr(mClusters[i]->GetHpr().x, 0, 0);
					mClusters[i]->mVelocity = mClusters[i]->RotateVector(Vector3(randX, randY, 30));
				}
			}
		
			if(mClusterTimer >= 3.0f)
			{
				mClusterActive = false;
				mClusterTimer = 0;
			}
		}

		CheckEnemyCollisions();

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

		for(unsigned i =0;i<mSpawnLocations.size();i++ )
		{
			if(Vector3::Distance(mPlayer.mPos,mSpawnLocations[i])<=50)
			{
				if(mSpawnDelay<1.5f)
				{
					mSpawnDelay+=Timer::GetDeltaTime();
				}
				else
				{
					mSpawnDelay = 0.0f;
				}
				if(mSpawnDelay==0.0f)
				{
					if(mBarracks[i]->IsAlive() == true)
						DeployEnemy(i);
				}
			}
		}

		UpdateAliveNodes(mEnemies);
		UpdateAliveNodes(mBullets);
		UpdateAliveNodes(mEnemyBullets);
		UpdateAliveNodes(mClusters);
		UpdateAliveNodes(mClusterAoE);

		for(unsigned i = 0;i<mEnemies.size();i++)
		{
			if(mEnemies[i]->IsAlive())
			{
				mEnemies[i]->mPos.y = mpTerrain->GetHeight(mEnemies[i]->mPos.x,mEnemies[i]->mPos.z) + 1.5f;
				/*if(mEnemies[i]->mPos.y>maxHeight)
				{
					mEnemies[i]->mPos.y = maxHeight;
					mEnemies[i]->mPos = mEnemies[i]->mPrevPos;
				}*/

				bool collide = false;

				for(unsigned e = 0;e<mEnemies.size();e++)
				{
					if(mEnemies[e]->IsAlive())
					{
						if(mEnemies[i]!=mEnemies[e])
						{
							BoundingSphere bs = mEnemies[i]->GetBounds();
							if(bs.Intersects(mEnemies[e]->GetBounds()))
							{
								collide = true;
							}
						}
					
						if(collide)
						{
							mEnemies[i]->mVelocity = -(mEnemies[e]->mPos - mEnemies[i]->mPos);
							mEnemies[e]->mVelocity = -(mEnemies[i]->mPos - mEnemies[e]->mPos);
						
							mEnemies[i]->mPos += mEnemies[i]->mVelocity*Timer::GetDeltaTime();
							//mEnemies[e]->mPos += mEnemies[e]->mVelocity*Timer::GetDeltaTime();
							mEnemies[i]->mTooNearBool = true;
							mEnemies[e]->mTooNearBool = true;
						
							//mEnemies[i]->mPos = mEnemies[i]->mPrevPos;
						}
						else
						{
							mEnemies[i]->mTooNearBool = false;
							mEnemies[e]->mTooNearBool = false;
						}
					}
				}
			}
		}

		//CheckCollisions();

		for(unsigned i = 0;i<mEnemies.size();i++)
		{
			if(mEnemies[i]->attackBool)
			{
				if(mEnemies[i]->shotTimer==0.0f)
				{
					FireEnemyShot(mEnemies[i]);
				}
			}
		}

		CheckPlayerCollisions();

		Input::SetMousePos(mCentreOfScreen.x, mCentreOfScreen.y, GetWindow());

		float MOVE_SPEED = 15;
		float TURN_SPEED = XMConvertToRadians(60);
		Vector3 move = GetKeyboardMovement(KBMOVE_WSAD);
		Vector3 turn = GetMouseTurn();
		turn.y = 0;

		mPlayer.Move(move*MOVE_SPEED*Timer::GetDeltaTime());
		mPlayer.mPos.y = mpTerrain->GetHeight(mPlayer.mPos.x, mPlayer.mPos.z) + 3;

		if(move==Vector3(0,0,0))
		{
			if(mVolume>0.1f)
			{
				mVolume -= 0.2f*Timer::GetDeltaTime();
			}
		}
		else
		{
			if(mVolume<=0.6f)
			{
				mVolume += 0.1f*Timer::GetDeltaTime();
			}
		}
		engine->setSoundVolume(mVolume);

		if(mPlayer.mPos.y > maxHeight)
			mPlayer.Move(-move*MOVE_SPEED*Timer::GetDeltaTime());

		PlayerCollision();

		mPlayer.Turn(turn*TURN_SPEED*Timer::GetDeltaTime());

		mCamera.SetPos(mPlayer.GetPos() + mPlayer.RotateVector(mCurrentCamMode));
		mCamera.LookAt(mPlayer.GetPos() + mPlayer.RotateVector(mCurrentCamAngle));
		//mCamera.LookAt(Vector3(0,0,3));
		//mCamera.Update();

		//// END GAME CONDITION
		// Check if everything except trees are destroyed
		for(int e = 0; e < mEnemies.size(); e++)
		{
			if(mEnemies[e]->IsAlive() == true)
			{
				mEndGameCond = false;
				break;
			}
			else
			{
				mEndGameCond = true;
			}
		}

		if(mEndGameCond)
		{
			for(int b = 0; b < mBarracks.size(); b++)
			{
				if(mBarracks[b]->IsAlive() == true)
				{
					mEndGameCond = false;
					break;
				}
				else
				{
					mEndGameCond = true;
				}
			}

			// Player wins
			if(mEndGameCond == true)
			{
				mEnergy = 0;
			}
		}
		////
	}
	else
	{
		engine->stopAllSounds();
		if(mhighScore<mScore)
		{
			mhighScore = mScore;
		}
		if(!isBoom)
		{
			ISound* missleHit = sfxEngine->play2D("../Content/Sounds/Missile-Hit.mp3",false);
			isBoom = true;
		}
		if(Input::KeyPress(VK_ESCAPE))
		{
			CloseWin();
		}
		if(Input::KeyPress(0x52))
		{
			ISound* UiClick = sfxEngine->play2D("../Content/Sounds/UI_Clicks09.wav",false);
			Restart();
		}
	}
}

void MyApp::Restart()
{
	mEnergy = 100.0f;
	mScore = 0;
	for(unsigned i =0;i<mBullets.size();i++)
	{
		mBullets[i]->Kill();
	}
	for(unsigned i =0;i<mEnemies.size();i++)
	{
		mEnemies[i]->Kill();
	}
	for(unsigned i =0;i<mClusters.size();i++)
	{
		mClusters[i]->Kill();
	}
	for(unsigned i =0;i<mEnemyBullets.size();i++)
	{
		mEnemyBullets[i]->Kill();
	}
	for(unsigned i =0;i<mClusterAoE.size();i++)
	{
		mClusterAoE[i]->Kill();
	}
	for(unsigned i =0;i<mBarracks.size();i++)
	{
		mBarracks[i]->Kill();
	}
	for(unsigned i =0;i<mTrees.size();i++)
	{
		mTrees[i]->Kill();
	}

	mSpawnLocations.clear();
	mSpawnLocations.push_back(Vector3(-100,mpTerrain->GetHeight(-100,10),10));
	mSpawnLocations.push_back(Vector3(-10,mpTerrain->GetHeight(-10,-100),-100));
	mSpawnLocations.push_back(Vector3(85,mpTerrain->GetHeight(85,-30),-30));
	mSpawnLocations.push_back(Vector3(90,mpTerrain->GetHeight(90,90),90));
	mSpawnLocations.push_back(Vector3(-60,mpTerrain->GetHeight(-60,60),60));

	SetupBarracks();
	SetupTrees();
	mPlayer.SetPos(Vector3(-80,0,-100));
	ISound* music = engine->play2D("../Content/Sounds/Tank.mp3",true);
	mVolume = 0.1f;
	engine->setSoundVolume(mVolume);
}

#pragma region Methods
void Bullet::Update()
{
	mLifetime-= Timer::GetDeltaTime();
	mPos+=mVelocity*Timer::GetDeltaTime();
	if(mLifetime<0)
	{
		Kill();//Kills self
	}
}

void Enemy::Update()
{
	if(Vector3::Distance((*mEnemyPos),mPos)>25)
	{
		
		if(mTooNearBool==false)
		{
			mVelocity = (*(mEnemyPos) - GetPos())/3;
		}
		//Move(-(mVelocity*Timer::GetDeltaTime()));
		mPos+=mVelocity*Timer::GetDeltaTime();
		attackBool = false;
		//mPos.y = MyApp::GetHeight(mPos);
	}
	else 
	{
		attackBool = true;
	}
	
	LookAt(*(mEnemyPos));
	if(mHealth<0)
	{
		Kill();
	}
	if(attackBool == true)
	{
		shotTimer+=Timer::GetDeltaTime();
	}
	if(shotTimer>1.5f)
	{
		shotTimer = 0.0f;
	}
}

void MyApp::DeployEnemy(int loc)
{
	Enemy* ptr =FindDeadNode(mEnemies);
	if(ptr == nullptr)return;

	ptr->attackBool = false;
	ptr->mHealth = 100.0f;
	ptr->mVelocity = mPlayer.GetPos() - ptr->mPos;
	//int i = rand() % (int)(4 - 0 + 1);
	//ptr->mPos = mSpawnLocations[i];
	ptr->mScale = 0.02f;
	ptr->SetPos(mSpawnLocations[loc] + Vector3(4,0,0));
	ptr->shotTimer = 0.0f;
	ptr->mEnemyPos = &mPlayer.mPos;
	ptr->mVelocity = (*(ptr->mEnemyPos) - ptr->GetPos())/3;
	ptr->mTooNearBool = false;
}
void MyApp::SetupBarracks()
{
	for(int i = 0; i < mSpawnLocations.size(); i++)
	{
		DrawableNode* ptr = FindDeadNode(mBarracks);
		if(ptr == nullptr)return;

		ptr->mHealth = 500.0f;
		ptr->mScale = 0.18f;
		ptr->SetPos(mSpawnLocations[i] + Vector3(0,1,0));
	}
}
void MyApp::SetupTrees()
{
	bool placable = false;

	for(int i = 0; i < mTrees.size(); i++)
	{
		DrawableNode* ptr = FindDeadNode(mTrees);
		if(ptr == nullptr)return;

		float randX = 0;
		float randZ = 0;
		float loc = 0;

		while(!placable)
		{
			randX = randf(-1000, 1000);
			randZ = randf(-1000, 1000);
			loc = mpTerrain->GetHeight(randX, randZ);

			if (loc != 0 && loc < maxHeight - 5)
			{
				placable = true;
			}
		}

		ptr->mHealth = 100.0f;
		ptr->mScale = 0.02f;
		ptr->SetPos(Vector3(randX,loc + 2,randZ));

		placable = false;
	}
}

void MyApp::FireShot()
{
	Bullet* ptr = FindDeadNode(mBullets);
	if(ptr== nullptr)return;
	sfxEngine->setSoundVolume(0.2f);
	ISound* shot = sfxEngine->play2D("../Content/Sounds/Body_Hit_31.wav",false);
	
	mEnergy--;
	ptr->mHealth=100;
	ptr->mLifetime= 3.0f;
	ptr->SetPos(mPlayer.GetPos() + mPlayer.RotateVector(Vector3(-1,1.2f,1)));
	ptr->mVelocity = mPlayer.RotateVector(Vector3(2.5f,-3.6f,20));
	ptr->mScale=0.4f;
}

void MyApp::FireEnemyShot(Enemy* enemy)
{
	Bullet* ptr = FindDeadNode(mEnemyBullets);
	if(ptr== nullptr)return;
	sfxEngine->setSoundVolume(0.2f);
	ISound* shot = sfxEngine->play2D("../Content/Sounds/Body_Hit_31.wav",false);

	//mEnergy--;
	ptr->mHealth=100;
	ptr->mLifetime= 3.0f;
	ptr->SetPos(enemy->mPos);
	ptr->mPos.y+=1;
	ptr->mVelocity =  mPlayer.mPos - enemy->mPos;
	ptr->mScale=0.4f;
}

void MyApp::FireCluster()
{
	/*
	Bullet* ptr = FindDeadNode(mBullets);
	if(ptr== nullptr)return;
	*/

	//float rot = 40;
	ISound* clustershot = sfxEngine->play2D("../Content/Sounds/Shotgun.mp3",false);
	ISound* missleLaunch = sfxEngine->play2D("../Content/Sounds/Missile-Launch.mp3",false);
	float rot = -70;
	for(int i = 0; i < mClusters.size(); i++)
	{
		//float rot = randf(-40, 40);
		Bullet* ptr = mClusters[i];
		ptr->mHealth=100;
		ptr->mLifetime= 2.9f;
		ptr->SetPos(mPlayer.GetPos() + mPlayer.RotateVector(Vector3(0,3,-0.8f)));
		//ptr->Yaw(mPlayer.GetHpr().z);
		ptr->SetHpr(mPlayer.GetHpr().x, 0, XMConvertToRadians(rot + i*20));
		ptr->mVelocity = mClusters[i]->RotateVector(Vector3(0,12,-50 * mClusterTimer));
		ptr->mScale=0.6f;
	}
	mEnergy-=10;
	engine->setSoundVolume(mVolume);
}
void MyApp::DisplayClusterAoE()
{
	for(int i = 0; i < mClusterAoE.size(); i++)
	{
		float rot = 360 / mClusterAoE.size() * i;
		Bullet* ptr = mClusterAoE[i];
		ptr->mLifetime = 5.0f;
		ptr->mHealth = 100;
		ptr->SetHpr(rot,0,0);
		ptr->SetPos(mPlayer.GetPos() + mPlayer.RotateVector(Vector3(0,0,15)) + mClusterAoE[i]->RotateVector(Vector3(0,0,3)));
		ptr->mPos.y = mpTerrain->GetHeight(mClusterAoE[i]->mPos.x, mClusterAoE[i]->mPos.z) + 0.2f;
		ptr->mScale = 0.5f;
	}
}
void MyApp::PlayerCollision()
{
	for(int i = 0; i < mEnemies.size(); i++)
	{
		if(mEnemies[i]->IsAlive()==false) continue;

		BoundingBox bb = mEnemies[i]->GetBox();

		if(bb.Intersects(mPlayer.GetBox()))
		{
			mEnemies[i]->Kill();
			mScore += 10;
		}
	}

	for(int i = 0; i < mBarracks.size(); i++)
	{
		if(mBarracks[i]->IsAlive()==false) continue;

		BoundingBox bb = mBarracks[i]->GetBox();

		if(bb.Intersects(mPlayer.GetBox()))
		{
			mBarracks[i]->Kill();
			//mSpawnLocations.erase(mSpawnLocations.begin() + i);
			mScore += 20;
		}
	}

	for(int i = 0; i < mTrees.size(); i++)
	{
		if(mTrees[i]->IsAlive()==false) continue;

		BoundingBox bb = mTrees[i]->GetBox();

		if(bb.Intersects(mPlayer.GetBox()))
		{
			mTrees[i]->Kill();
			mScore += 1;
		}
	}
}
void MyApp::CheckEnemyCollisions()
{
	// Regular bullet
	for(int b = 0;b<mBullets.size();b++)
	{
		if(mBullets[b]->IsAlive()==false) continue;
		
		BoundingBox bb = mBullets[b]->GetBox();
		
		// Enemies
		for(int t = 0;t < mEnemies.size();t++)
		{
			if(mEnemies[t]->IsAlive()==false)continue;

			if(bb.Intersects(mEnemies[t]->GetBox()))
			{
				mBullets[b]->Kill();
				mEnemies[t]->mHealth-=mPlayerDamage;
				mScore += 10;

				if(mEnemies[t]->mHealth <= 0)
				{
					mScore += 20;
				}
			}
		}

		// Barracks
		for(int t = 0;t < mBarracks.size();t++)
		{
			if(mBarracks[t]->IsAlive()==false)continue;

			if(bb.Intersects(mBarracks[t]->GetBox()))
			{
				mBullets[b]->Kill();
				mBarracks[t]->mHealth-=mPlayerDamage;
				mScore += 20;

				if(mBarracks[t]->mHealth <= 0)
				{
					mScore += 100;
					mBarracks[t]->Kill();
					//mSpawnLocations.erase(mSpawnLocations.begin() + t);
				}
			}
		}

		// Trees
		for(int t = 0;t < mTrees.size();t++)
		{
			if(mTrees[t]->IsAlive()==false)continue;

			if(bb.Intersects(mTrees[t]->GetBox()))
			{
				mBullets[b]->Kill();
				mTrees[t]->mHealth-=mPlayerDamage;
				mScore += 1;

				if(mTrees[t]->mHealth <= 0)
				{
					mScore += 5;
				}
			}
		}
	}

	// Cluster
	if(mClusterActive)
	{
		for(int c = 0; c < mClusters.size(); c++)
		{
			if(mClusters[c]->IsAlive()==false) continue;
		
			BoundingBox bb = mClusters[c]->GetBox();

			// Enemies
			for(int t = 0;t < mEnemies.size();t++)
			{
				if(mEnemies[t]->IsAlive()==false)continue;

				if(bb.Intersects(mEnemies[t]->GetBox()))
				{
					mClusters[c]->Kill();
					mEnemies[t]->mHealth-=25;

					// prevent player from overscoring
					if(mEnemies[t]->mHealth >= 0)
						mScore += 10;

					if(mEnemies[t] <= 0)
						mEnemies[t]->Kill();
				}
			}

			// Barracks
			for(int t = 0;t < mBarracks.size();t++)
			{
				if(mBarracks[t]->IsAlive()==false)continue;

				if(bb.Intersects(mBarracks[t]->GetBox()))
				{
					mClusters[c]->Kill();
					mBarracks[t]->mHealth-=mPlayerDamage;

					// prevent player from overscoring
					if(mBarracks[t]->mHealth >= 0)
						mScore += 20;

					if(mBarracks[t]->mHealth <= 0)
					{
						mBarracks[t]->Kill();
						//mSpawnLocations.erase(mSpawnLocations.begin() + t);
					}
				}
			}

			// Trees
			for(int t = 0;t < mTrees.size();t++)
			{
				if(mTrees[t]->IsAlive()==false)continue;

				if(bb.Intersects(mTrees[t]->GetBox()))
				{
					mClusters[c]->Kill();
					mTrees[t]->mHealth-=mPlayerDamage;

					// prevent player from overscoring
					if(mTrees[t]->mHealth >= 0)
						mScore += 1;

					if(mTrees[t]->mHealth <= 0)
						mTrees[t]->Kill();
				}
			}
		}
	}
}

void MyApp::CheckPlayerCollisions()
{
	for(int b = 0;b<mEnemyBullets.size();b++)
	{
		if(mEnemyBullets[b]->IsAlive()==false) continue;
		
		BoundingBox bb = mEnemyBullets[b]->GetBox();

		if(bb.Intersects(mPlayer.GetBox()))
		{
			mEnemyBullets[b]->Kill();
			mRecoverDelay = 0;
			sfxEngine->play2D("../Content/Sounds/Body_Hit_11.wav",false);
			mEnergy -= 5;
			//mPlayer.mHealth -= 25;
		}
	}
}
#pragma endregion

void MyApp::Shutdown()
{
	DeleteAllNodes(mBullets);
	DeleteAllNodes(mEnemies);
	DeleteAllNodes(mClusters);
	DeleteAllNodes(mEnemyBullets);
	DeleteAllNodes(mClusterAoE);
	DeleteAllNodes(mBarracks);
	DeleteAllNodes(mTrees);
	engine->drop();
	sfxEngine->drop();
}

// in console C++ is was main()
// in Windows C++ its called WinMain()  (or sometimes wWinMain)
int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine,
	int nShowCmd)
{
	/*bool stopbool =false;
	int goInt = 0;
	do
	{*/
	ENABLE_LEAK_DETECTION();
	MyApp app;
	return app.Go(hInstance);	// go!
	/*}while(!stopbool);
	return goInt;*/
}



