/******************************
* Hello IGRA!
* Written by Some Asian Dude (- A -)/
******************************/

#include "DxCommon.h"	// common DX stuff

#include "IgraApp.h"	// the IGRA 'game' class
#include <DirectXColors.h>
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include <IgraUtils.h>
#include "ToString.h"
#include "SimpleMathToString.h"
#include "D3DCompiler.h"
#pragma comment(lib, "D3DCompiler.lib")

using namespace igra;
using namespace DirectX;
using namespace DirectX::SimpleMath;

struct Vertex
{
	Vector3 pos;
	Color col;
};

// vertex layout
// note: the offsetof() command to get the correct byte offset in the structure
const D3D11_INPUT_ELEMENT_DESC VERTEX_LAYOUT[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, col), D3D11_INPUT_PER_VERTEX_DATA, 0},
};
const UINT VERTEX_LAYOUT_LEN = ARRAYSIZE(VERTEX_LAYOUT);

// this is the constant buffer(cbuffer) from VS_Coloured3d.hlsl
// this MUST MATCH THE SHADER EXACTLY
// if it doesn't it will fail
struct cbPerObject
{
	Matrix WVP; // world matrix * view matrix * projection matrix
};

class MyApp : public App
{
public:
	virtual void Startup();
	virtual void Update();
	virtual void Draw();
	//virtual void Shutdown();

private:
	Color mBackground;

	void MakeVertexBuffer();
	void LoadShaders();

	CComPtr<ID3D11Buffer> mpVertexBuffer;
	CComPtr<ID3D11Buffer> mpIndexBuffer;
	int mNumIndex;
	CComPtr<ID3D11VertexShader> mpVertexShader;
	CComPtr<ID3D11PixelShader> mpPixelShader;
	CComPtr<ID3D11InputLayout> mpInputLayout;

	CComPtr<ID3D11Buffer> mpcbPerObjectBuffer; // the DX buffer
	cbPerObject mcbPerObject; // the struct to be filled

	// Smart pointer to CommonStates;
	std::unique_ptr<CommonStates> mpCommonStates;

	Vector3 mCamPos;
	float mCamSpeed;
	float cubeRotationA, cubeRotationB, cubeRotationC, cubeRotationD;

	float cirY;
	float cirZ;
};

void MyApp::Startup()
{
	mTitle=L"This is Task01a";
	App::Startup(); // call the base class

	std::wstring useThis =L"Hello World";

	mBackground = (XMVECTOR)DirectX::Colors::Aquamarine;

	mCamPos = Vector3(0,0,-16);
	mCamSpeed = 5;

	cubeRotationA = 0;
	cubeRotationB = 0;
	cubeRotationC = 0;
	cubeRotationD = 0;

	cirY = 8 * cos(40);
	cirZ = 8 * sin(40);

	mpCommonStates.reset(new CommonStates(GetDevice()));

	MakeVertexBuffer();
	LoadShaders();
}

void MyApp::MakeVertexBuffer()
{
	/*
	Vertex verts[]={
		{Vector3(-0.5f,-0.5f,+0.5f), Colors::Green},
		{Vector3(-0.5f,+0.5f,+0.5f), Colors::Blue},
		{Vector3(+0.5f,+0.5f,+0.5f), Colors::Red},

		{Vector3(+0.5f,+0.5f,+0.5f), Colors::Yellow},
		{Vector3(+0.5f,-0.5f,+0.5f), Colors::BlueViolet},
		{Vector3(-0.5f,-0.5f,+0.5f), Colors::AntiqueWhite},

		{Vector3(-0.8f,-0.8f,+0.5f), Colors::Black},
		{Vector3(+0.8f,-0.8f,+0.5f), Colors::Black},
		{Vector3(+0.8f,+0.8f,+0.5f), Colors::Black},
	};
	*/
	Vertex verts[] =
	{
		{Vector3( -1, -1, -1 ),Colors::Black},
		{Vector3( 1, -1, -1 ),Colors::Red},
		{Vector3( -1, 1, -1 ),Colors::Green},
		{Vector3(  1, 1, -1 ),Colors::Yellow},

		{Vector3( -1, -1, 1 ),Colors::Blue},
		{Vector3( 1, -1, 1 ),Colors::Magenta},
		{Vector3( -1, 1, 1 ),Colors::Cyan},
		{Vector3( 1, 1, 1 ),Colors::White},
	};

	// size of the buffer in BYTES
	unsigned vertsSizeInBytes = sizeof(Vertex) * ARRAYSIZE(verts);

	mpVertexBuffer.Attach(CreateVertexBuffer(GetDevice(), verts, vertsSizeInBytes));

	// the index buffer data
	UINT indices[]=
	{
		0,3,1,	// Tri 1
		0,2,3,	// Tri 2
		2,6,7,	// Tri 3
		2,7,3,	// Tri 4
		1,3,7,
		1,7,5,
		1,5,0,
		0,5,4,
		0,4,2,
		2,4,6,
		4,5,6,
		5,7,6,
	};
	mNumIndex=ARRAYSIZE(indices);// number of indexes (needed for rendering)

	mpIndexBuffer.Attach(CreateIndexBuffer(GetDevice(), indices, mNumIndex));

	// create constant buffer:
	D3D11_BUFFER_DESC cbbd;	
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR(GetDevice()->CreateBuffer(&cbbd, NULL, &mpcbPerObjectBuffer));
}

void MyApp::LoadShaders()
{
	// a blob is like a BYTE[]
	// it's used in DX for arbitary lumps of binary data
	CComPtr<ID3DBlob> pBlob;
	// load the cso object
	HR(D3DReadFileToBlob(L"../Debug/VS_Coloured3D.cso", &pBlob));
	// load cso to make vertex shader
	HR(GetDevice()->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &mpVertexShader));
	// load cso to make input layout
	HR(GetDevice()->CreateInputLayout(VERTEX_LAYOUT, VERTEX_LAYOUT_LEN, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &mpInputLayout));

	// empty the pBlob
	pBlob.Release();

	// load the cso
	HR(D3DReadFileToBlob(L"../Debug/PS_Coloured.cso", &pBlob));
	// make the pixel shader
	HR(GetDevice()->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &mpPixelShader));
}

void MyApp::Update()
{
	if(cubeRotationA >= 36000)
		cubeRotationA = 0;
	if(cubeRotationB >= 36000)
		cubeRotationB=0;
	if(cubeRotationC >= 36000)
		cubeRotationB=0;
	if(cubeRotationD >= 36000)
		cubeRotationB=0;

	cubeRotationA += 60 * Timer::GetDeltaTime();
	cubeRotationB += 150 * Timer::GetDeltaTime();
	cubeRotationC += 80 * Timer::GetDeltaTime();
	cubeRotationD += 120 * Timer::GetDeltaTime();

	// Exit program
	if (Input::KeyPress(VK_ESCAPE))
	{
		CloseWin();
	}
	
	if(Input::KeyDown(VK_RIGHT))
	{
		mCamPos.x -= mCamSpeed * Timer::GetDeltaTime();
	}
	if(Input::KeyDown(VK_LEFT))
	{
		mCamPos.x += mCamSpeed * Timer::GetDeltaTime();
	}
	if(Input::KeyDown(VK_UP))
	{
		mCamPos.y -= mCamSpeed * Timer::GetDeltaTime();
	}
	if(Input::KeyDown(VK_DOWN))
	{
		mCamPos.y += mCamSpeed * Timer::GetDeltaTime();
	}
	if(Input::KeyDown(VK_NUMPAD2))
	{
		mCamPos.z -= mCamSpeed * Timer::GetDeltaTime();
	}
	if(Input::KeyDown(VK_NUMPAD8))
	{
		mCamPos.z += mCamSpeed * Timer::GetDeltaTime();
	}

	// change game state
	if(Input::KeyPress(VK_F1))
	{
		GetContext()->RSSetState(mpCommonStates->CullNone());
		mTitle =L"No Culling";
	}
	if(Input::KeyPress(VK_F2))
	{
		GetContext()->RSSetState(mpCommonStates->CullClockwise());
		mTitle =L"Cull Clockwise";
	}
	if(Input::KeyPress(VK_F3))
	{
		GetContext()->RSSetState(mpCommonStates->CullCounterClockwise());
		mTitle =L"Cull Counter Clockwise(default)";
	}
	if(Input::KeyPress(VK_F4))
	{
		GetContext()->RSSetState(mpCommonStates->Wireframe());
		mTitle =L"Wireframe";
	}
}

void MyApp::Draw()
{
	// Clear our backbuffer
	GetContext()->ClearDepthStencilView(GetDepthStencilView(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1, 0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(), Colors::SkyBlue);

	// set topology
	GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// set input layout
	GetContext()->IASetInputLayout(mpInputLayout);

	// set VS & PS
	GetContext()->VSSetShader(mpVertexShader, nullptr, 0);
	GetContext()->PSSetShader(mpPixelShader, nullptr, 0);

	// setup the camera
	Vector3 camTgt(0, 0, 0);
	Matrix world = Matrix::Identity();
	Matrix view = XMMatrixLookAtLH(mCamPos, camTgt, Vector3(0, 1, 0));
	Matrix proj = XMMatrixPerspectiveFovLH(0.4f * XM_PI, GetAspectRatio(), 0.1f, 50);

#pragma region Center cube
	//world = Matrix::CreateTranslation(2.5f, 0, 0);
	world = Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationA));
	// fill the constant bffer
	Matrix WVP = world * view * proj;
	// all matrixes must be transposed
	mcbPerObject.WVP = WVP.Transpose();
	//copy to the GPU
	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	ID3D11Buffer* cbuffers[]={mpcbPerObjectBuffer};
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);

	// set vertex buffer
	UINT stride = sizeof( Vertex ); // size of 1x vertex in bytes
	UINT offset = 0;
	ID3D11Buffer* buffers[]={mpVertexBuffer}; // array of VB pointers
	GetContext()->IASetVertexBuffers( 0, 1, buffers, &stride, &offset );

	// set index buffer
	GetContext()->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	GetContext()->DrawIndexed(mNumIndex, 0, 0);
#pragma endregion

#pragma region First Orbit
	//setup a new world matrix
	world = Matrix::CreateTranslation(-4.0f, 0, 0) * Matrix::CreateRotationX(XMConvertToRadians(cubeRotationB)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationB));
	
	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);
	
	//setup a third world matrix
	world = Matrix::CreateTranslation(4.0f, 0, 0) * Matrix::CreateRotationX(XMConvertToRadians(cubeRotationB)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationB));
	
	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);
#pragma endregion

#pragma region Second orbit
	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationB)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationB))
		* world = Matrix::CreateTranslation(0, -8.0f, 0)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, -8.0f, 0);
	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);

	////////////////

	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationC)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationC))
		* Matrix::CreateTranslation(0, 8.0f, 0)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, 8.0f, 0);
	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);

	////////////////

	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationD)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationD)) 
		* Matrix::CreateTranslation(0, 0, -8.0f)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, 0, -8.0f);
	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);

	////////////////

	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationC)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationB))
		* Matrix::CreateTranslation(0, 0, 8.0f)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, 0, 8.0f);

	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);

	// Cube 5

	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationB)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationD))
		* Matrix::CreateTranslation(0, cirY, -cirZ)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, 0, 8.0f);

	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);

	// Cube 6

	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationC)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationD))
		* Matrix::CreateTranslation(0, -cirY, cirZ)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, 0, 8.0f);

	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);

	// Cube 7

	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationB)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationC))
		* Matrix::CreateTranslation(0, -cirY, -cirZ)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, 0, 8.0f);

	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);

	// Cube 8

	world = Matrix::CreateRotationZ(XMConvertToRadians(cubeRotationD)) * Matrix::CreateRotationY(XMConvertToRadians(cubeRotationB))
		* Matrix::CreateTranslation(0, cirY, cirZ)
		* Matrix::CreateRotationX(XMConvertToRadians(cubeRotationA));
	//world = Matrix::CreateTranslation(0, 0, 8.0f);

	WVP = world * view *proj;
	mcbPerObject.WVP = WVP.Transpose();

	GetContext()->UpdateSubresource(mpcbPerObjectBuffer, 0, NULL, &mcbPerObject, 0, 0);
	GetContext()->VSSetConstantBuffers(0, 1, cbuffers);
	GetContext()->DrawIndexed(mNumIndex, 0, 0);
#pragma endregion

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
