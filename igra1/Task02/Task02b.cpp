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
	CComPtr<ID3D11VertexShader> mpVertexShader;
	CComPtr<ID3D11PixelShader> mpPixelShader;
	CComPtr<ID3D11InputLayout> mpInputLayout;

	// Smart pointer to CommonStates;
	std::unique_ptr<CommonStates> mpCommonStates;
};

void MyApp::Startup()
{
	mTitle=L"This is Task01a";
	App::Startup(); // call the base class

	std::wstring useThis =L"Hello World";

	mBackground = (XMVECTOR)DirectX::Colors::Aquamarine;

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

	Vertex verts[48];
	
	Vertex center = {Vector3(0.0f, 0.0f, 0.5f), Colors::Blue};
	int inc = 1;
	int index = 0;

	float point = 0.1f;
	for(index = 0; index < 48;)
	{
		if(index < 24)
		{
			Vertex tempB = {Vector3(0.1f, 0.5f, 0.1f * inc), Colors:: Yellow};
			Vertex tempC = {Vector3(point + 0.1f, 0.1f * inc, 0.5f), Colors::Purple};	
		
			inc++;
			if(point < 0.6f)
			{
				point += 0.1f;
			}

			verts[index] = center;
			index++;
			verts[index] = tempB;
			index++;
			verts[index] = tempC;
			index++;
		}
		else
		{
			if(inc > index/3)
			{
				inc = 1;
				point = 0.1f;
			}

			Vertex tempB = {Vector3(-point -0.1f, 0.1f * inc, 0.5f), Colors::Purple};
			Vertex tempC = {Vector3(-0.1f, 0.5f, 0.1f * inc), Colors::Yellow};
		
			inc++;
			if(-point > -0.6f)
			{
				point += 0.1f;
			}

			verts[index] = center;
			index++;
			verts[index] = tempB;
			index++;
			verts[index] = tempC;
			index++;	
		}
	}

	// size of the buffer in BYTES
	unsigned vertsSizeInBytes = sizeof(Vertex) * 48;
	// to create to VB we need a couple of structs filled
	D3D11_BUFFER_DESC buffDesc;
	ZeroMemory(&buffDesc, sizeof(buffDesc)); // clear it
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.ByteWidth = vertsSizeInBytes;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	
	D3D11_SUBRESOURCE_DATA subResData;
	ZeroMemory(&subResData, sizeof(subResData));
	subResData.pSysMem = (void*)verts; // set the pointer to the array

	// to the actual creation
	GetDevice()->CreateBuffer(&buffDesc, &subResData, &mpVertexBuffer);
}

void MyApp::LoadShaders()
{
	// a blob is like a BYTE[]
	// it's used in DX for arbitary lumps of binary data
	CComPtr<ID3DBlob> pBlob;
	// load the cso object
	HR(D3DReadFileToBlob(L"../Debug/VS_Coloured.cso", &pBlob));
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
	// Exit program
	if (Input::KeyPress(VK_ESCAPE))
	{
		CloseWin();
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

	// set vertex buffer
	UINT stride = sizeof( Vertex ); // size of 1x vertex in bytes
	UINT offset = 0;
	ID3D11Buffer* buffers[]={mpVertexBuffer}; // array of VB pointers
	GetContext()->IASetVertexBuffers( 0, 1, buffers, &stride, &offset );

	// Draw
	GetContext()->Draw(48, 0);

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
