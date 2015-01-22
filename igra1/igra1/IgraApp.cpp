/*==============================================
 * IGRA Application classes
 *
 * Written by Mark Gossage
 *==============================================*/
#pragma once

#include <Windowsx.h>	// windows messages

#include "IgraApp.h"// main class

namespace igra
{

/*********************************************************************
* WndProc for the App
*********************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Input::WndProc(hWnd,message,wParam,lParam);	// give to input first
	switch (message) 
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

App::App()
{
	mTitle=L"Igra App v4.0";
	// default window size
	mSize.left=mSize.top=0;
	mSize.right=1024;
	mSize.bottom=768;
	// set all to null
	mpSwapChain=nullptr;
	mpD3d11Device=nullptr;
	mpD3d11DevCon=nullptr;
	mpRenderTargetView=nullptr;
	mpDepthStencilView=nullptr;
	mpDepthStencilBuffer=nullptr;
}

App::~App()
{
}

void App::Startup(){}
void App::Update(){}
void App::Draw(){}
void App::Shutdown(){}

void App::CloseWin()
{
	// this seems ok for now
	SendMessage(mHWnd,WM_CLOSE,0,0);
	// or use:
	// PostQuitMessage(0);
}

bool App::OpenWin()
{
	const TCHAR CLASS_NAME[]=TEXT("IGRA App");

	// adjust width/height for the window style
	RECT r=mSize;
	AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,false);
	int width=r.right-r.left;
	int height=r.bottom-r.top;

	// the WNDCLASS
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_CLASSDC;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetInstance();
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= CLASS_NAME;
	wcex.hIconSm		= 0;
	RegisterClassEx(&wcex);

	// create the window
	mHWnd = CreateWindow(CLASS_NAME, 
							 mTitle.c_str(), 
							 WS_OVERLAPPEDWINDOW,
							 CW_USEDEFAULT, 
							 CW_USEDEFAULT, 
							 width, 
							 height, 
							 NULL, 
							 NULL, 
							 GetInstance(), 
							 NULL);

	if (!mHWnd)
		return false;

	ShowWindow(mHWnd, SW_SHOWDEFAULT);
	UpdateWindow(mHWnd);
	return true;	
}
bool App::SetupDirectX()
{
	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
	bufferDesc.Width = mSize.right;
	bufferDesc.Height = mSize.bottom;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc; 
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	// 4X MSAA
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	//swapChainDesc.SampleDesc.Count = 4;
	//swapChainDesc.SampleDesc.Quality = 2;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = GetWindow(); 
	swapChainDesc.Windowed = TRUE; 
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//Create our SwapChain
	HR(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &mpSwapChain, &mpD3d11Device, NULL, &mpD3d11DevCon));

	//Create our BackBuffer
	ID3D11Texture2D* pBackBuffer;
	HR(mpSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ));

	//Create our Render Target
	HR(mpD3d11Device->CreateRenderTargetView( pBackBuffer, NULL, &mpRenderTargetView ));
	pBackBuffer->Release();

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width     = mSize.right;
	depthStencilDesc.Height    = mSize.bottom;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count   = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;
	// create the Depth/Stencil 
	HR(mpD3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &mpDepthStencilBuffer));
	HR(mpD3d11Device->CreateDepthStencilView(mpDepthStencilBuffer, NULL, &mpDepthStencilView));

	//Set our Render Target & Depth/Stencil 
	mpD3d11DevCon->OMSetRenderTargets( 1, &mpRenderTargetView, mpDepthStencilView );


	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)mSize.right;
	viewport.Height = (float)mSize.bottom;
	viewport.MinDepth=0;
	viewport.MaxDepth=1;

	//Set the Viewport
	GetContext()->RSSetViewports(1, &viewport);

	return true;
}

void App::CloseDirectX()
{
	// Release the COM Objects we created
	if (mpSwapChain)	mpSwapChain->Release();
	if (mpD3d11Device)	mpD3d11Device->Release();
	if (mpD3d11DevCon)	mpD3d11DevCon->Release();
	if (mpRenderTargetView)		mpRenderTargetView->Release();
	if (mpDepthStencilBuffer)	mpDepthStencilBuffer->Release();
	if (mpDepthStencilView)		mpDepthStencilView->Release();
}

int App::MessageLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	WCHAR title[1024];
	while(true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		}
		else
		{
			// internal code 
			Input::Update();
			if (Timer::Update())
			{
				// update the FPS display
				//swprintf(title,1024,L"%s @ %.2f Hz %.2f ms",mTitle.c_str(),Timer::GetFrameRate(),1000.f*Timer::GetSmoothedFrameTime());
				swprintf(title,1024,L"%s @ %.2f Hz %.2f ms",mTitle.c_str(),Timer::GetFrameRate(),1000.f*Timer::GetDeltaTime());
				SetWindowText(mHWnd,title);
			}
			// run game logic  
			Update();
			Draw();
		}
	}
	return msg.wParam;
}

int App::Go(HINSTANCE hInstance)
{
	mInstance=hInstance;	// store instance
	Timer::Startup();

	if (!OpenWin())
	{
		FAIL(L"Create Window - Failed");
	}

	if(!SetupDirectX())	//Initialize Direct3D
	{
		FAIL(L"Direct3D Initialization - Failed");
	}

	Startup();	//Initialize our scene

	int result=MessageLoop();

	// shutdown (not checking for results)
	Shutdown();
	CloseDirectX();    

	return result;
}

void App::Clear(const float* rgba)
{
	GetContext()->ClearDepthStencilView(mpDepthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
	GetContext()->ClearRenderTargetView(GetRenderTargetView(), rgba);
}

void App::Present()
{
	GetSwapChain()->Present(0, 0);
}

void App::DebugLog(const char* str)
{
	OutputDebugStringA(str);
}
void App::DebugLog(const wchar_t* str)
{
	OutputDebugStringW(str);
}
void App::DebugLog(const std::wstring& str)
{
	OutputDebugStringW(str.c_str());
}

////////////////////////////////////////////////////////////////////////////////////
// Input Class
////////////////////////////////////////////////////////////////////////////////////

// static variables
POINT Input::mMousePos={0,0},Input::mMouseOld={0,0},Input::mMouseDelta={0,0};
int Input::mMouseZ=0,Input::mMouseW=0;
bool Input::mKeyDown[256]={0},Input::mOldKeyDown[256]={0};

LRESULT CALLBACK Input::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_MOUSEMOVE:	// capture mouse input information
			mMousePos.x = GET_X_LPARAM(lParam); 
			mMousePos.y = GET_Y_LPARAM(lParam);
			break;
		case WM_MOUSEWHEEL:	// capture mouse wheel information
			//mMousePos.x = GET_X_LPARAM(lParam); 
			//mMousePos.y = GET_Y_LPARAM(lParam);
			mMouseZ = GET_WHEEL_DELTA_WPARAM(wParam);
			break;
	default:
		break;
	}
	return 0;
}

void Input::Update()
{
	// update keyboard state:
	// GetAsyncKeyState seems the best for this 
	for(int i=0;i<256;i++)
	{
		mOldKeyDown[i]=mKeyDown[i];
		mKeyDown[i]=(GetAsyncKeyState(i)<0);	// quick way to check if MSB is set
	}
	// update mouse
	mMouseDelta.x=mMousePos.x-mMouseOld.x;
	mMouseDelta.y=mMousePos.y-mMouseOld.y;
	mMouseOld=mMousePos;
	mMouseW=mMouseZ;	// the mouse wheel value
	mMouseZ=0;	// clear the wheel counter
}

void Input::SetMousePos(int x, int y,HWND theWnd)
{
	mMouseOld.x=x;	mMouseOld.y=y;	// update the old mouse pos variable for delta tracking
	// this is a bit messy as we have to include the size of the window & borders
	RECT r;
	::GetWindowRect(theWnd,&r);
	x+=r.left; y+=r.top;	//apply window offset
	r.left=0;	r.top=0;
	AdjustWindowRect(&r,GetWindowLong(theWnd,GWL_STYLE),false);
	x-=r.left;	y-=r.top;	// apply window style offset
	SetCursorPos(x,y);
}

////////////////////////////////////////////////////////////////////////////////////
// Timer Class
////////////////////////////////////////////////////////////////////////////////////
// statics:
LARGE_INTEGER Timer::mTimerFreq,Timer::mTimerStart;
unsigned Timer::mFrameCount=0;		///< count of frames
float Timer::mTime=0,Timer::mDeltaTime=0,Timer::mFrameRate=0,Timer::mFrameTime=0,Timer::mLastUpdate=0;

void Timer::Startup()
{
	// init the high performance timer
	QueryPerformanceFrequency(&mTimerFreq);
	QueryPerformanceCounter(&mTimerStart);
	// reset timer vars
	mTime=mDeltaTime=mFrameRate=mLastUpdate=0;
	mFrameCount=0;
}

bool Timer::Update()
{
	float oldTime=mTime;	// prev time
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);	// get time now:
	mTime=(float)(now.QuadPart-mTimerStart.QuadPart)/mTimerFreq.QuadPart;
	mDeltaTime=mTime-oldTime;
	// update the frame time (smoothed)
	mFrameTime+=0.1f*(mDeltaTime-mFrameTime);
	// frame rate
	const float FPS_UPDATE_TIME=0.25f;	// how often to update
	mFrameCount++;
	if (mTime>=mLastUpdate+FPS_UPDATE_TIME)	// time for update frame rate
	{
		float fps=mFrameCount/(mTime-mLastUpdate);	// frame rate
		//mFrameRate+=(0.1f)*(fps-mFrameRate);	// smoothed update
		mFrameRate=fps;	// exact rate
		mFrameCount=0;
		mLastUpdate=mTime;
		return true;
	}
	return false;
}

float Timer::GetAbsoluteTime()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);	// get time now:
	return (float)(now.QuadPart)/mTimerFreq.QuadPart;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug code
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FAIL_MSG(HRESULT hr,const char* strMsg,const char* strFile,DWORD dwLine)
{
	const int BUFFER_SIZE=3000;
    WCHAR strBufferLine[128];
    WCHAR strBufferError[256];
    WCHAR strBuffer[BUFFER_SIZE];

	// error code:
	if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,0,hr,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),strBufferError,256,0)<1)
		lstrcpyW(strBufferError,L"Unknown error");
	// line number
	if (strFile==nullptr)
		lstrcpyW(strBufferLine,L"");
	else
		swprintf(strBufferLine,128,L"File: %hs\nLine: %d\n",strFile,dwLine);
	// the whole message
	swprintf(strBuffer,BUFFER_SIZE,L"%sError Code: 0x%X\n%s%hs\nDo you want to debug the application?",strBufferLine,hr,strBufferError,strMsg);

	// debug print
	OutputDebugStringW(strBuffer);
	// messagebox
	if (MessageBoxW(GetForegroundWindow(),strBuffer, L"Unexpected error encountered", MB_YESNO | MB_ICONERROR )== IDYES)
        DebugBreak();
}

void FAIL_MSG(HRESULT hr,const WCHAR* strMsg,const char* strFile,DWORD dwLine)
{
	const int BUFFER_SIZE=3000;
    WCHAR strBufferLine[128];
    WCHAR strBufferError[256];
    WCHAR strBuffer[BUFFER_SIZE];

	// error code:
	if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,0,hr,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),strBufferError,256,0)<1)
		lstrcpyW(strBufferError,L"Unknown error");
	// line number
	if (strFile==nullptr)
		lstrcpyW(strBufferLine,L"");
	else
		swprintf(strBufferLine,128,L"File: %hs\nLine: %d\n",strFile,dwLine);
	// the whole message
	swprintf(strBuffer,BUFFER_SIZE,L"%sError Code: 0x%X\n%s%s\nDo you want to debug the application?",strBufferLine,hr,strBufferError,strMsg);

	// debug print
	OutputDebugStringW(strBuffer);
	// messagebox
	if (MessageBoxW(GetForegroundWindow(),strBuffer, L"Unexpected error encountered", MB_YESNO | MB_ICONERROR )== IDYES)
        DebugBreak();
}


}	// namespace