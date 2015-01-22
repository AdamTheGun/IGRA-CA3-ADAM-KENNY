/*==============================================
 * IGRA Application classes
 *
 * Written by IGRA Team
 *==============================================*/
#pragma once

#include "DxCommon.h"	// Common Dx stuff

// link appropriate libraries 
#pragma comment(lib, "d3d11.lib")

namespace igra
{

/** The igra::App class wrappers the Win32 window & DX11.

This class holds all the main functionality needed for opening up a window
of the desired size & the main DX11 classes

\par References:
- Wendy Jones (or any other DirectX book) for the basic window stuff
- http://winprog.org/tutorial for a much more detailed Windows tutorial
- MSDN for details on capturing all the mouse messages
 - http://msdn.microsoft.com/en-us/library/ms645617%28VS.85%29.aspx mouse wheel code
 - http://msdn.microsoft.com/en-us/library/ms645616%28VS.85%29.aspx mouse move

*/
class App
{
public:
	/** Constructor, does effectivly nothing.
	the only code added here is screen size.
	All initalisation code should be in Startup()
	*/
	App();
	/// Destructor, unused
	/// Don't put any code in here, put it inside Shutdown()
	virtual ~App();
	/** main game engine loop.
	Basically what this does is call Startup,Update,Render & Shutdown in that order.
	All your code should be added into one of these functions.
	You shouldn't need to modify the Go function, but just in case...
	*/
	virtual int Go(HINSTANCE hInstance);
protected:	// functions for overriding
	/** The user startup function, All your startup code should go here.
	*/
	virtual void Startup();
	/** The user update function, all game logic goes here.
	*/
	virtual void Update();
	/** The user draw function, all display goes here.
	The code you have should include the normal Clear, Present flow that you find in Direct3D applications.
	*/
	virtual void Draw();
	/** the user shutdown function, all shutdown code goes here.
	Your code should SAFE_DELETE or SAFE_RELEASE all the objects required
	*/
	virtual void Shutdown();

public:	// control functions
	/// closes the window, causes the app to exit
	void CloseWin();

	std::wstring mTitle;		///< the window title, can be set directly

	/** returns the window handle.
	This is useful when certain applications require the windows handle 
	in their initalisation.
	*/
	HWND GetWindow(){return mHWnd;}
	/// The instance. Just in case you need it...
	HINSTANCE GetInstance(){return mInstance;}
	/// Returns a rectangle of the window size.
	/// Useful when you need to make an image be stretched into the window.
	RECT GetWindowRect(){return mSize;}
	/// Returns the aspect ratio for the window.
	float GetAspectRatio(){return (float(mSize.right))/mSize.bottom;}

	/// clears screen & depth buffer at beginning of drawing
	void Clear(const float* rgba);
	/// updates screen at end of drawing
	void Present();

	// accessors for DX
	ID3D11Device* GetDevice(){ return mpD3d11Device;}
	ID3D11DeviceContext* GetContext(){return mpD3d11DevCon;}
	IDXGISwapChain* GetSwapChain(){ return mpSwapChain;}
	ID3D11RenderTargetView* GetRenderTargetView(){return mpRenderTargetView;}
	ID3D11DepthStencilView* GetDepthStencilView(){return mpDepthStencilView;}

	// helpful logging routine:
	static void DebugLog(const char* str);
	static void DebugLog(const wchar_t* str);
	static void DebugLog(const std::wstring& str);

private:	// internal functions
	/// creates the window
	bool OpenWin();
	/// sets up DX11
	bool SetupDirectX();
	/// the message loop
	int MessageLoop();
	/// closes all DX
	void CloseDirectX();

protected:
	RECT mSize;	///< the window size

private:
	HWND mHWnd;	///< the window handle
	HINSTANCE mInstance;			///< the instance
	// DX items
	ID3D11Device* mpD3d11Device;
	ID3D11DeviceContext* mpD3d11DevCon;
	IDXGISwapChain* mpSwapChain;
	ID3D11RenderTargetView* mpRenderTargetView;
	ID3D11DepthStencilView* mpDepthStencilView;
	ID3D11Texture2D* mpDepthStencilBuffer;
private:
	App(const App&);	/// NO COPYING
	void operator=(const App&);	/// NO NO COPYING
};

/** Input routines
These routine rely on the normal Win32 API's to get the mouse and keyboard input.
They are all static, so that they can be called from anywhere.
\code
if (Input::KeyPress('A'))
	...
\endcode
*/
class Input
{
public:
	/** returns true if a given key is down.
	\param vk the keycode to use: which could be
	- 'A'..'Z','0'..'9' for keypresses
	- VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN for cursor keys
	- VK_NEXT, VK_PRIOR, VK_HOME, VK_END for page down, page up, home, end 
	- VK_ENTER, VK_TAB, VK_ESCAPE, VK_SPACE as expected
	- VK_LBUTTON, VK_MBUTTON, VK_RBUTTON for mouse buttons
	- and many others (see WinUser.h or http://msdn.microsoft.com/en-us/library/ms927178.aspx)
	*/
	static inline bool KeyDown(byte vk){return mKeyDown[vk];}
	/** returns true if a given key has been pressed in the past game cycle.
	\param vk the keycode to use: see KeyPress() for a list of keycodes
	*/
	static inline bool KeyPress(int vk){return mKeyDown[vk] && !mOldKeyDown[vk];}

	/** Returns the mouses location in pixels.
	\returns POINT x,y the mouses location, relative to the window
	\note mouse tracking becomes irregular once the mouse if outside of the window
	*/
	static POINT GetMousePos(){return mMousePos;}
	/** Returns amount mouse has moved.
	\returns POINT x,y amount mouse has moved (usually <100 pixels)
	\note mouse tracking becomes irregular once the mouse if outside of the window
	*/
	static POINT GetMouseDelta(){return mMouseDelta;}
	
	/** returns the mouse wheel value.
	normally this is either -120,0 or +120
	*/
	static int GetMouseWheel(){return mMouseW;}

	/** Sets the mouses location relative to the window:
	\param x,y the location to put the mouse
	*/
	static void SetMousePos(int x, int y,HWND theWnd);

	/** The WndProc for the input handler.
	\internal Don't use this, its automatically called by Igra::App
	*/
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	/** Called once per cycle to update the internal logic.
	\internal Don't use this, its automatically called by Igra::App
	*/
	static void Update();
private:
	static POINT mMousePos,mMouseOld,mMouseDelta;	// mouse data
	static int mMouseZ,mMouseW;	// mouse Z data
	static bool mKeyDown[256],mOldKeyDown[256];	// keyboard data
};

/** High Resolution Timer.
If its functions looks a little like the one from Unity 3D, well....
*/
class Timer
{
public:
	/// returns the number of seconds since start of App
	static float GetTime(){return mTime;}
	/// returns number of seconds since last frome (will be ~1/60)
	static float GetDeltaTime(){return mDeltaTime;}
	/// returns the frame rate (smoothed)
	static float GetFrameRate(){return mFrameRate;}
	/// returns the frame rate (smoothed)
	static float GetSmoothedFrameTime(){return mFrameTime;}

	/** Sets up the class
	\internal its called by the App class, you don't need to
	*/
	static void Startup();
	/** Updates the timer. Must be called once per frame to make it work properly.
	\return whether the FPS has been updated
	\internal its called by the App class, you don't need to
	*/
	static bool Update();
	/** returns the absolute time.
	This is not relative to anything. */
	static float GetAbsoluteTime();
private:
	static LARGE_INTEGER mTimerFreq;	///< timer frequency information
	static LARGE_INTEGER mTimerStart;	///< beginning of the timer
	static float mTime,mDeltaTime;		///< abs time, delta time
	static unsigned mFrameCount;		///< count of frames
	static float mFrameRate,mFrameTime;	///< current frame rate/time
	static float mLastUpdate;			///< previous computation of frame rate
};



#define FAIL(msg) igra::FAIL_MSG(E_FAIL,msg,__FILE__,__LINE__)
#define FAIL_HR(hr) igra::FAIL_MSG(hr,0,__FILE__,__LINE__)
#define FAIL_HR_MSG(hr,msg) igra::FAIL_MSG(hr,msg,__FILE__,__LINE__)

#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			igra::FAIL_MSG(hr, L#x, __FILE__, (DWORD)__LINE__);      \
		}                                                      \
	}
	#define HR_MSG(x,msg)                                      \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			igra::FAIL_MSG(hr, msg, __FILE__, (DWORD)__LINE__);      \
		}                                                      \
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#define HR_MSG(x,msg) (x)
	#endif
#endif 

// 2 versions: ascii or unicode/wide
void FAIL_MSG(HRESULT hr,const WCHAR* strMsg,const char* strFile,DWORD dwLine);
void FAIL_MSG(HRESULT hr,const char* strMsg,const char* strFile,DWORD dwLine);

}
