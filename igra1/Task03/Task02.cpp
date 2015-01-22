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
};

// vertex layout
// note: the offsetof() command to get the correct byte offset in the structure
const D3D11_INPUT_ELEMENT_DESC VERTEX_LAYOUT[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex,pos),
                  D3D11_INPUT_PER_VERTEX_DATA, 0 },  
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
};

void MyApp::Startup()
{
	mTitle=L"This is Task01a";
	App::Startup(); // call the base class

	std::wstring useThis =L"Hello World";

	mBackground = (XMVECTOR)DirectX::Colors::Aquamarine;

	MakeVertexBuffer();
	LoadShaders();
}

void MyApp::MakeVertexBuffer()
{
	Vertex verts[]={
		{Vector3(-0.5f,-0.5f,-0.5f)},
		{Vector3(-0.5f,+0.5f,-0.5f)},
		{Vector3(+0.5f,+0.5f,+0.5f)},

		{Vector3(+0.5f,+0.5f,+0.5f)},
		{Vector3(+0.5f,-0.5f,+0.5f)},
		{Vector3(-0.5f,-0.5f,+0.5f)},
	};

	// size of the buffer in BYTES
	unsigned vertsSizeInBytes = sizeof(Vertex) * 6;
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
	HR(D3DReadFileToBlob(L"../Debug/VertexShader.cso", &pBlob));
	// load cso to make vertex shader
	HR(GetDevice()->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &mpVertexShader));
	// load cso to make input layout
	HR(GetDevice()->CreateInputLayout(VERTEX_LAYOUT, VERTEX_LAYOUT_LEN, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &mpInputLayout));

	// empty the pBlob
	pBlob.Release();

	// load the cso
	HR(D3DReadFileToBlob(L"../Debug/PixelShader.cso", &pBlob));
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
	GetContext()->Draw(6, 0);

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
