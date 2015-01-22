/*==============================================
 * IGRA Utilities
 *
 * Written by <YOUR NAME HERE>
 *==============================================*/

#include "IgraUtils.h"
#include "IgraApp.h"

#include "WICTextureLoader.h"	// texture loader routines
#include "DDSTextureLoader.h"	// texture loader routines

using namespace DirectX::SimpleMath;

namespace igra
{
ID3D11Buffer* CreateVertexBuffer(ID3D11Device* pDev, const void* data,unsigned dataBytes)
{
	//FAIL("TODO: CreateVertexBuffer()");
	// go back and look at your previous labtasks on how to create a vertex buffer
	ID3D11Buffer* pBuffer=nullptr;

	// to create to VB we need a couple of structs filled
	D3D11_BUFFER_DESC buffDesc;
	ZeroMemory(&buffDesc, sizeof(buffDesc)); // clear it
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.ByteWidth = dataBytes;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	
	D3D11_SUBRESOURCE_DATA subResData;
	ZeroMemory(&subResData, sizeof(subResData));
	subResData.pSysMem = (void*)data; // set the pointer to the array

	HR(pDev->CreateBuffer( &buffDesc, &subResData,
		&pBuffer));

	return pBuffer;
}

ID3D11Buffer* CreateIndexBuffer(ID3D11Device* pDev,const UINT indices[],unsigned ind_len)
{
	//FAIL("TODO: CreateIndexBuffer()");
	// go back and look at your previous labtasks on how to create a index buffer
	ID3D11Buffer* pBuffer=nullptr;

	// to create to VB we need a couple of structs filled
	D3D11_BUFFER_DESC buffDesc;
	ZeroMemory(&buffDesc, sizeof(buffDesc)); // clear it
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.ByteWidth = sizeof(UINT)*ind_len;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	
	D3D11_SUBRESOURCE_DATA subResData;
	ZeroMemory(&subResData, sizeof(subResData));
	subResData.pSysMem = (void*)indices; // set the pointer to the array

	HR(pDev->CreateBuffer( &buffDesc, &subResData,
		&pBuffer));

	return pBuffer;
}

ID3D11ShaderResourceView* CreateTextureResourceWIC(ID3D11Device* pDev,const wchar_t* filename)
{
	ID3D11ShaderResourceView* pTex;
	HR_MSG(DirectX::CreateWICTextureFromFile(pDev,filename,nullptr,&pTex),filename);
	return pTex;
}

ID3D11ShaderResourceView* CreateTextureResourceDDS(ID3D11Device* pDev,const wchar_t* filename)
{
	ID3D11ShaderResourceView* pTex;
	HR_MSG(DirectX::CreateDDSTextureFromFile(pDev,filename,nullptr,&pTex),filename);
	return pTex;
}

// code from http://www.d3dcoder.net/phpBB/viewtopic.php?f=3&t=346
// modified to work with WIC textures
ID3D11ShaderResourceView* CreateTexture2DArray(ID3D11Device* d3dDEvice, ID3D11DeviceContext* context,
                                         const std::vector<std::wstring>& filesname)
{
  unsigned textureCount = filesname.size();
  std::vector<ID3D11Texture2D*> vResources2D(textureCount);
  
  //Create a 2DTexture for each texture file
  for(unsigned i = 0; i < textureCount; ++i)
  {
   //Note: This function is in the DirectX Tool Kit Library
   HR_MSG(DirectX::CreateWICTextureFromFileEx(d3dDEvice, 
                        filesname[i].c_str(), 
                        NULL,
                        D3D11_USAGE_IMMUTABLE,//You can also use D3D11_USAGE_DEFAULT
                        D3D11_BIND_SHADER_RESOURCE,
                        NULL,
                        NULL, 
                        false, 
                        reinterpret_cast<ID3D11Resource**>(&vResources2D[i]),
                        nullptr),	//Ensure the last parameter is NULL so a view to the texture is not created!
						filesname[i].c_str());	// name of image (for error report)
  }
 
  //Get the description from at least 1 of the textures since both HAVE to be the same dimension and format
  D3D11_TEXTURE2D_DESC vtexDesc;
  vResources2D[0]->GetDesc(&vtexDesc);
  
  D3D11_TEXTURE2D_DESC texDesc;  
  texDesc.Width              = vtexDesc.Width;
  texDesc.Height           = vtexDesc.Height;
  texDesc.MipLevels           = vtexDesc.MipLevels;
  texDesc.ArraySize           = textureCount;
  texDesc.Format           = vtexDesc.Format;
  texDesc.SampleDesc.Count     = 1;
  texDesc.SampleDesc.Quality  = 0;
  texDesc.Usage              = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags     = NULL;
  texDesc.MiscFlags           = NULL;
  
  ID3D11Texture2D* textureArray = nullptr;
  HR(d3dDEvice->CreateTexture2D(&texDesc, NULL, &textureArray));

  //Per texture
  for(unsigned itextureIndex = 0; itextureIndex < textureCount; ++itextureIndex)
  {
   //Per MipMap level
   for(unsigned miplevel = 0; miplevel < texDesc.MipLevels; miplevel++)
   {
     context->CopySubresourceRegion(textureArray,
                           D3D11CalcSubresource(miplevel, itextureIndex, texDesc.MipLevels),
                           NULL,
                           NULL,
                           NULL,
                           vResources2D[itextureIndex],
                           miplevel,
                           nullptr);
   }
  }
  //Release the temporary Video Memory used per texture 
  for(unsigned i = 0; i < textureCount; ++i)
   vResources2D[i]->Release();
 
  //Create the view to the Texture array by filling out the Texture2DArray union from D3D11_SHADER_RESOURCE_VIEW_DESC
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  srvDesc.Format = texDesc.Format;
  srvDesc.ViewDimension   = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
  srvDesc.Texture2DArray.MostDetailedMip = 0;
  srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
  srvDesc.Texture2DArray.FirstArraySlice = 0;
  srvDesc.Texture2DArray.ArraySize = textureCount;
  
  ID3D11ShaderResourceView* TextureSRV = nullptr;
  HR(d3dDEvice->CreateShaderResourceView(textureArray, &srvDesc, &TextureSRV));
  
  textureArray->Release();

  //Remember that the pointer returned here needs to be properly release by YOUR application
  return TextureSRV;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
DirectX::SimpleMath::Ray ScreenPointToRay(const POINT& mouse,const RECT& screen,const DirectX::SimpleMath::Matrix& view,const DirectX::SimpleMath::Matrix& proj)
{
	// copied DIRECTLY from
	// http://www.toymaker.info/Games/html/picking.html
	// you want to know how it works? Read the site...
	DirectX::SimpleMath::Vector3 v;
	v.x =  ( ( ( 2.0f * mouse.x ) / (screen.right-screen.left)  ) - 1 ) / proj._11;
	v.y = -( ( ( 2.0f * mouse.y ) / (screen.bottom-screen.top) ) - 1 ) / proj._22;
	v.z =  1.0f;

	DirectX::SimpleMath::Matrix m=view.Invert();	// inverse the view matrix

	// Transform the screen space pick ray into 3D space
	DirectX::SimpleMath::Vector3 rayDir(v.x*m._11 + v.y*m._21 + v.z*m._31,
										v.x*m._12 + v.y*m._22 + v.z*m._32,
										v.x*m._13 + v.y*m._23 + v.z*m._33);
	rayDir.Normalize();	// just to be sure
	DirectX::SimpleMath::Vector3 rayPos(m._41,m._42,m._43);
	return DirectX::SimpleMath::Ray(rayPos,rayDir);
}

bool CollideRayMesh(const DirectX::SimpleMath::Ray& ray,const std::vector<DirectX::SimpleMath::Vector3>& triangles,float& dist)
{
	dist=FLT_MAX;
	bool hit=false;
	for(unsigned i=0;i<triangles.size()-2;i+=3)
	{
		float d=FLT_MAX;
		if (ray.Intersects(triangles[i],triangles[i+1],triangles[i+2],d))
		{
			hit=true;
			if (dist>d)	dist=d;
		}
	}
	return hit;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

Vector3 GetKeyboardMovement(const int keys[])
{
	Vector3 res(0,0,0);
	if (keys[0]!=0 && Input::KeyDown(keys[0]))	res.x--;
	if (keys[1]!=0 && Input::KeyDown(keys[1]))	res.x++;
	if (keys[2]!=0 && Input::KeyDown(keys[2]))	res.y--;
	if (keys[3]!=0 && Input::KeyDown(keys[3]))	res.y++;
	if (keys[4]!=0 && Input::KeyDown(keys[4]))	res.z--;
	if (keys[5]!=0 && Input::KeyDown(keys[5]))	res.z++;
	return res;
}

Vector3 GetMouseTurn()
{
	POINT mv=Input::GetMouseDelta();
	return Vector3((float)(mv.x),(float)mv.y,0);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void ArcBallCamera::Reset()
{
	mDist=5;
	mYaw=mPitch=0;
	ComputeMatrix();
}
void ArcBallCamera::Update()
{
	const float WHEEL_SPEED=0.01f;
	const float TURN_SPEED_X=DirectX::XM_PI/50;
	const float TURN_SPEED_Y=TURN_SPEED_X/3;
	const float MAX_ANGLE=DirectX::XMConvertToRadians(85);
	POINT mv=Input::GetMouseDelta();
	mDist+=WHEEL_SPEED*Input::GetMouseWheel();
	if (mDist<1)	mDist=1;
	mYaw+=mv.x*TURN_SPEED_X;
	mPitch+=mv.y*TURN_SPEED_Y;
	if (mPitch>MAX_ANGLE) mPitch=MAX_ANGLE;
	if (mPitch<-MAX_ANGLE) mPitch=-MAX_ANGLE;

	ComputeMatrix();
}
DirectX::SimpleMath::Matrix ArcBallCamera::GetDefaultProjectionMatrix()
{
	// using LH rather than RH as all the DX books are LH based
	return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90),4.0f/3.0f,0.1f,10000.0f);
}

void ArcBallCamera::SetAngles(float yaw,float pitch)
{
	mYaw=yaw; 
	mPitch=pitch;
	ComputeMatrix();
}

void ArcBallCamera::SetFocus(const Vector3& p)
{
	mFocus=p;
	ComputeMatrix();
}

void ArcBallCamera::ComputeMatrix()
{
	// set camera pos & matrix
	Matrix rot=Matrix::CreateRotationX(mPitch)*Matrix::CreateRotationY(mYaw);
	// we just need to get the rotation matrix & transform the up & forward Vector3's
	Vector3 up(0,1,0),forward(0,0,1);
	up=Vector3::TransformNormal(up,rot);	// up=up*rot;
	forward=Vector3::TransformNormal(forward,rot);	// back=back*rot;
	mCamPos=mFocus-(forward*mDist);	
	// using LH rather than RH as all the DX books are LH based
	mView=DirectX::XMMatrixLookAtLH(mCamPos,mFocus,up);

}

///////////////////////////////////////////////////////////////////////////////
/// given a filename+path, this extracts the filename
std::wstring ExtractFilename(const std::wstring& path)
{
	std::wstring::size_type pos=path.find_last_of(L"\\");
	if (pos==std::wstring::npos)
		pos=path.find_last_of(L"/");
	if (pos==std::wstring::npos)
		return path;
	return path.substr(pos+1);
}

/// given a filename+path, this extracts the path
std::wstring ExtractPath(const std::wstring& path)
{
	std::wstring::size_type pos=path.find_last_of(L"\\");
	if (pos==std::wstring::npos)
		pos=path.find_last_of(L"/");
	if (pos==std::wstring::npos)
		return L"";
	return path.substr(0,pos);
}
/// given a path & filename, combines them, adding the '/' if needed
std::wstring CombinePath(const std::wstring& path,const std::wstring& filename)
{
	if (path.length()<=0)	return filename;	// no path
	wchar_t end=path[path.length()-1];
	if (end=='/' || end=='\\')	// already has the path seperator
		return path+filename;
	return path+L'/'+filename;	// add seperator
}
/// given a filename, this extracts the name without extension
std::wstring RemoveFileExtention(const std::wstring& filename)
{
	std::wstring::size_type pos=filename.find_last_of(L".");
	if (pos==std::wstring::npos)
		return filename;
	return filename.substr(0,pos);
}

float randf(){return ((float)rand())/RAND_MAX;}
float randf(float a, float b){return randf()*(b-a)+a;}

}	// namespace