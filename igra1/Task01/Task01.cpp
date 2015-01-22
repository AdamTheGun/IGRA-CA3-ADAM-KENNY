/******************************
* Hello IGRA!
* Written by Some Asian Dude (- A -)/
******************************/

#include "DxCommon.h"	// common DX stuff

#include "IgraApp.h"	// the IGRA 'game' class

using namespace igra;

// in console C++ is was main()
// in Windows C++ its called WinMain()  (or sometimes wWinMain)
int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine,
	int nShowCmd)
{
	App app;
	app.mTitle=L"Hello IGRA";	
		// set the title (its L"..." because of unicode)
	return app.Go(hInstance);	// go!
}
