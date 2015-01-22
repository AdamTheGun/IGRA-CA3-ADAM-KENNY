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

using namespace igra;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class MyApp : public App
{
public:
	virtual void Startup();
	virtual void Update();
	virtual void Draw();
	//virtual void Shutdown();

private:
	Color mBackground;

	float pos;

	float r,g,b,a;
	int swap;

	Vector2 imgPos;
	float accX;
	float accY;

	std::unique_ptr<SpriteBatch> mpSpriteBatch;
	std::unique_ptr<SpriteFont> mpSpriteFont;
	CComPtr<ID3D11ShaderResourceView> mpTexLogo;
};

void MyApp::Startup()
{
	mTitle=L"This is Task01a";
	App::Startup(); // call the base class

	std::wstring useThis =L"Hello World";

	mBackground = (XMVECTOR)DirectX::Colors::Aquamarine;

	swap = 0;
	r=0;
	g=0;
	b=0;
	a=1;

	accX = 0.15;
	accY = 0.1;
	imgPos = Vector2(100, 100);

	mpSpriteBatch.reset(new SpriteBatch(GetContext()));
	mpSpriteFont.reset(new SpriteFont(GetDevice(), L"../Content/Times12.sprfont"));
	mpTexLogo = CreateTextureResourceWIC(GetDevice(), L"../Content/Troll.png");
}

void MyApp::Update()
{
	// Image movement
	imgPos.x += accX;
	imgPos.y += accY;

	if(imgPos.x >= GetWindowRect().right - 128)
	{
		accX = -0.15;
	}
	else if(imgPos.y >= GetWindowRect().bottom - 128)
	{
		accY = -0.1;
	}
	else if(imgPos.x <= GetWindowRect().left)
	{
		accX = 0.15;
	}
	else if(imgPos.y <= GetWindowRect().top)
	{
		accY = 0.1;
	}
	
	// Background colour code
	if(swap == 0)
	{
		if(r < 1)
		{
			r += 0.0001f;
		}
		else if(g < 1)
		{
			g += 0.0001f;
		}
		else if(b < 1)
		{
			b += 0.0001f;
		}
		else
		{
			swap = 1;
		}
	}
	if(swap == 1)
	{
		if(b > 0)
		{
			b -= 0.0001f;
		}
		else if(g > 0)
		{
			g -= 0.0001f;
		}
		else if(r > 0)
		{
			r -= 0.0001f;
		}
		else
		{
			swap = 0;
		}
	}

	if(Input::KeyPress('R'))
	{
		mBackground = (XMVECTOR)Colors::Red;
	}
	else if(Input::KeyPress('G'))
	{
		mBackground = (XMVECTOR)Colors::Green;
	}
	else if(Input::KeyPress('A'))
	{
		mBackground = (XMVECTOR)Colors::Aquamarine;
	}

	// Exit program
	if (Input::KeyPress(VK_ESCAPE))
	{
		CloseWin();
	}
}

void MyApp::Draw()
{// Clear our backbuffer
	float mColour[] = {r,g,b,a};
	GetContext()->ClearRenderTargetView(GetRenderTargetView(), mColour);
	mpSpriteBatch->Begin();

	std::wstring str = ToString("Pos is ", imgPos);

	mpSpriteFont->DrawString(mpSpriteBatch.get(), str.c_str(), Vector2(20,20), Colors::Black);
	mpSpriteBatch->Draw(mpTexLogo, imgPos);
	mpSpriteBatch->End();	

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
