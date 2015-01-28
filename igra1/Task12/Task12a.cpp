/******************************
* Task11a.cpp
* Written by <Your name here>
******************************/

#include "DxCommon.h"	// common DX stuff
#include <DirectXColors.h>	// DirectX::Colors

#include "IgraApp.h"	// the IGRA 'game' class
#include "IgraUtils.h"	// useful functions
#include "ShaderManager.h"	// holds all the shaders

#include "ModelObj.h"	// the IGRA model loader
#include "Node.h"	// 3d object managment

#include "ToString.h"	// helpful string convertion
#include "SimpleMathToString.h"	// helpful string convertion

#include "GeometryGenerator.h"	// 3d object managment


using namespace igra;
using namespace DirectX;	// XMVECTOR
using namespace DirectX::SimpleMath;	// for Color



class MyApp:public App
{
	void Startup();
	void Update();
	void Draw();
	void Shutdown();	

	// shader manager: handles VS,PS, layouts & constant buffers
	std::unique_ptr<ShaderManager> mpShaderManager;

	ModelObj mMainObj;
	std::vector<DrawableNode*> mModels;

	bool mUseCulling;
	bool mBackFace;

	unsigned DrawAllModels(const std::vector<DrawableNode*>& models,const Matrix& view,const Matrix& projection);
	unsigned DrawAllModelsCull(const std::vector<DrawableNode*>& models,const Matrix& view,const Matrix& projection,const BoundingFrustum& frust);

	ArcBallCamera mCamera;
};

void MyApp::Startup()
{
	// just create the Shader Manager
	mpShaderManager.reset(new ShaderManager(GetDevice()));

	float time=Timer::GetAbsoluteTime();
	mMainObj.Load(GetDevice(),mpShaderManager.get(),
                        L"../Content/dragon1k.obj",true);
	time=Timer::GetAbsoluteTime()-time;
	DebugLog(ToString("Load time: ",time*1000,"ms \n"));

	
	// create a dummy material for the model:
	mMainObj.mMaterial.mMaterial.gMaterial=MakeMaterial(Colors::Gray);
	mMainObj.mMaterial.SetupDefaultLights();
	mMainObj.mMaterial.mpPS=mpShaderManager->PSDirLight();

	// create 1000 models to be drawn!
	// in a box -50..+50
	for(int i=0;i<1000;i++)
	{
		float x=10*(i%10-5);
		float y=10*(i/10%10-5);
		float z=10*(i/100%10-5);
		DrawableNode* pNode=new DrawableNode();
		pNode->Init(&mMainObj,Vector3(x,y,z));
		mModels.push_back(pNode);
	}

	// setup camera
	mCamera.Reset();

	mUseCulling=false;
	mBackFace=false;
}

void MyApp::Update()
{
	if (Input::KeyPress(VK_ESCAPE))
		CloseWin();
	if (Input::KeyPress(VK_F1)) mUseCulling=false;
	if (Input::KeyPress(VK_F2)) mUseCulling=true;

	if (Input::KeyPress(VK_F5)) mBackFace=false;
	if (Input::KeyPress(VK_F6)) mBackFace=true;

	mCamera.Update();

}


BoundingFrustum CreateFrustum(const Matrix& projection,Vector3 pos,Vector3 hpr)
{
	BoundingFrustum cameraFrustum;
	BoundingFrustum::CreateFromMatrix(cameraFrustum,projection);	// the base camera
	Matrix trans=Matrix::CreateFromYawPitchRoll(hpr.x,hpr.y,hpr.z)*Matrix::CreateTranslation(pos);
	cameraFrustum.Transform(cameraFrustum,trans);
	return cameraFrustum;
}


void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(), Colors::SkyBlue);

	// Set common rendering flags
	GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	GetContext()->OMSetDepthStencilState(mpShaderManager->CommonStates()->DepthDefault(),0);
	ShaderManager::SetSampler(GetContext(),mpShaderManager->CommonStates()->LinearWrap());

	if (mBackFace)
		GetContext()->RSSetState(mpShaderManager->CommonStates()->CullCounterClockwise());
	else
		GetContext()->RSSetState(mpShaderManager->CommonStates()->CullNone());



	// setup the matrixes
	Matrix world;
	Matrix view=mCamera.GetViewMatrix();
	Matrix proj=mCamera.GetDefaultProjectionMatrix();

	BoundingFrustum frust=CreateFrustum(proj,mCamera.GetCamPos(),mCamera.GetCamAngles());

	// draw All the models!!!
	mTitle=L"Task 12a: ";
	unsigned count=0;
	if (mUseCulling)
	{
		count=DrawAllModelsCull(mModels,view,proj,frust);
		mTitle+=L"DrawAllModelsCull";
	}
	else
	{
		count=DrawAllModels(mModels,view,proj);
		mTitle+=L"DrawAllModels";
	}
	mTitle+=ToString(L" Models drawn ",count,"/",mModels.size());
	mTitle+=ToString(L" Cull:",mBackFace);

	// Present the backbuffer to the screen
	GetSwapChain()->Present(0, 0);
}
unsigned MyApp::DrawAllModels(const std::vector<DrawableNode*>& models,const Matrix& view,const Matrix& projection)
{
	unsigned count=0;
	for(unsigned i=0;i<models.size();++i)
	{
		models[i]->Draw(GetContext(),view,projection);
		count++;
	}
	return count;
}

unsigned MyApp::DrawAllModelsCull(const std::vector<DrawableNode*>& models,const Matrix& view,const Matrix& projection,const BoundingFrustum& frust)
{
	unsigned count=0;
	for(unsigned i=0;i<models.size();++i)
	{
		// must run 100% bounds just to be sure all are shown
		if (frust.Intersects(models[i]->GetBounds(1)))
		{
			models[i]->Draw(GetContext(),view,projection);
			count++;
		}
	}
	return count;
}

void MyApp::Shutdown()
{
	DeleteAllNodes(mModels);
}


// in console C++ is was main()
// in Windows C++ its called WinMain()  (or sometimes wWinMain)
int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine,
	int nShowCmd)
{
	ENABLE_LEAK_DETECTION();	// look for the leaks

	MyApp app;
	return app.Go(hInstance);	// go!
}
