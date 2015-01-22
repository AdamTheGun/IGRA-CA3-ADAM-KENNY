/*==============================================
 * IGRA Utilities
 *
 * Written by <YOUR NAME HERE>
 *==============================================*/
#pragma once

#include "DxCommon.h"	// Common Dx stuff

/** IGRA Utils is a collection of DX 11 functions & Constants.
Its has some dependency upon IgraApp, but is mainly stand alone.

*/


namespace igra
{
/** Creates a vertex buffer using a block of data.
\param pDev the DX11 device
\param data a pointer to the vertex data
\param dataBytes length of data in BYTES
\return the new vertex buffer.
	
Best usage is:
\code
CComPtr<ID3D11Buffer> mpVertexBuffer;	// vertex buffer
	
mpVertexBuffer.attach(CreateVertexBuffer(....));
// note: don't use mpVertexBuffer=CreateVertexBuffer(), it might leak memory
\endcode
*/	
ID3D11Buffer* CreateVertexBuffer(ID3D11Device* pDev, const void* data,unsigned dataBytes);

/** Creates a index buffer using a array of unsigned integers.
\param pDev the DX11 device
\param indices array of indexes
\param ind_len length of data in UNITS
\return the new index buffer.
	
Best usage is:
\code
CComPtr<ID3D11Buffer> mpIndexBuffer;	// index buffer
	
mpIndexBuffer.attach(CreateIndexBuffer(....));
// note: don't use mpIndexBuffer=CreateIndexBuffer(), it might leak memory
\endcode
*/	
ID3D11Buffer* CreateIndexBuffer(ID3D11Device* pDev,const UINT indices[],unsigned ind_len);


/** Loads a texture from BMP,JPG,PNG file (not DDS or PSD)
*/
ID3D11ShaderResourceView* CreateTextureResourceWIC(ID3D11Device* pDev,const wchar_t* filename);

/** Loads a texture DDS 
*/
ID3D11ShaderResourceView* CreateTextureResourceDDS(ID3D11Device* pDev,const wchar_t* filename);

/** Loads a texture array from BMP,JPG,PNG file (not DDS or PSD)
*/
ID3D11ShaderResourceView* CreateTexture2DArray(ID3D11Device* pDev, ID3D11DeviceContext* pContext,
                                         const std::vector<std::wstring>& filesname);

/** basic movement
note the order, -x,+x,-y,+y,-z+z
for movement you want z(forward), x(left) & possibly y(up)

eg of usage
\code
	// moves lightPos by 10 units/second
	lightPos+=GetKeyboardMovement(KBMOVE_CURSOR_PG)*Timer::GetDeltaTime()*10;
\endcode
*/
const int KBMOVE_WSAD[]={'A','D',0,0,'S','W'};
const int KBMOVE_WSADRF[]={'A','D','F','R','S','W'};
const int KBMOVE_CURSOR[]={VK_LEFT,VK_RIGHT,0,0,VK_DOWN,VK_UP};
const int KBMOVE_CURSOR_PG[]={VK_LEFT,VK_RIGHT,VK_NEXT,VK_PRIOR,VK_DOWN,VK_UP};
// for turning you want x(yaw) & y (pitch), but probably not z(roll)
const int KBMOVE_CURSOR_HRP[]={VK_LEFT,VK_RIGHT,VK_UP,VK_DOWN,0,0};///< for turning 

/// returns a vector of movement based upon the keys passed in
DirectX::SimpleMath::Vector3 GetKeyboardMovement(const int keys[]);
/// returns a vector for turning (XY only) based upon the mouse movement
DirectX::SimpleMath::Vector3 GetMouseTurn();

DirectX::SimpleMath::Ray ScreenPointToRay(const POINT& mouse,const RECT& screen,const DirectX::SimpleMath::Matrix& view,const DirectX::SimpleMath::Matrix& proj);
bool CollideRayMesh(const DirectX::SimpleMath::Ray& ray,const std::vector<DirectX::SimpleMath::Vector3>& triangles,float& dist);

class ArcBallCamera
{
public:
	ArcBallCamera(){Reset();}
	void Reset();
	DirectX::SimpleMath::Matrix GetViewMatrix(){return mView;}
	DirectX::SimpleMath::Vector3 GetCamPos(){return mCamPos;}
	DirectX::SimpleMath::Vector3 GetCamAngles(){return DirectX::SimpleMath::Vector3(mYaw,mPitch,0);}
	/// gets a generic 4:3 projection matrix
	DirectX::SimpleMath::Matrix GetDefaultProjectionMatrix();

	void Update();
	void SetAngles(float yaw,float pitch);
	void SetFocus(const DirectX::SimpleMath::Vector3& p);
	float mDist;
private:
	void ComputeMatrix();
	float mYaw,mPitch;
	DirectX::SimpleMath::Matrix mView;
	DirectX::SimpleMath::Vector3 mCamPos,mFocus;
};


/// given a filename+path, this extracts the filename
std::wstring ExtractFilename(const std::wstring& path);
/// given a filename+path, this extracts the path
std::wstring ExtractPath(const std::wstring& path);
/// given a path & filename, combines them, adding the '/' if needed
std::wstring CombinePath(const std::wstring& path,const std::wstring& filename);
/// given a filename, this extracts the name without extension
std::wstring RemoveFileExtention(const std::wstring& filename);


float randf();	///< random number 0..1
float randf(float a, float b);	///< random number a..b

/** Code to report memory leaks.
Simply add this code at the beginning of Winmain
\code
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine, int nCmdShow)
{
	ENABLE_LEAK_DETECTION();	// add here

	MyGame eng;
	return eng.Go(hInstance);	// runs everything
}
\endcode
At the end it will report memory leaks.
It does not report COM leaks (DirectX stuff), just normal C++ stuff.
*/
#define ENABLE_LEAK_DETECTION()\
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );\
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW );

}