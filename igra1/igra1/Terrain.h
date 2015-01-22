/*==============================================
 * Terrain Heightmap class
 *
 * Written by <YOUR NAME>
 *==============================================*/


#include "DxCommon.h"

namespace igra
{

class Terrain
{
public:
	Terrain(ID3D11Device* pDev,const wchar_t* filename,const DirectX::XMFLOAT3& scale=DirectX::XMFLOAT3(1,1,1));
	~Terrain();

	/// sets VB, IB & performs the draw call.
	/// all shaders, buffers, textures, samplers must be set before calling this
	void Draw(ID3D11DeviceContext* pContext);

	/// given the x,z points, returns the y height
	float GetHeight(float x, float z);
	/// given a point, gives a point which is offset usings above the terrain
	DirectX::XMFLOAT3 GetPosOnTerrain(DirectX::XMFLOAT3 pos,float offset=0);

private:	// internal functions
	// loads the bmp file
	void HeightMapLoad(const wchar_t* filename);
	// generates VB
	ID3D11Buffer* CreateTerrainVertexBuffer(ID3D11Device* pDev);
	// generates IB
	ID3D11Buffer* CreateTerrainIndexBuffer(ID3D11Device* pDev);
private:	// internal data
	ID3D11Device* mpDevice;
	int mTerrainWidth;		// Width of heightmap
	int mTerrainHeight;		// Height (Length) of heightmap
	std::vector<float> mHeightMap;	// Array to store terrain's height (size is mTerrainWidth * mTerrainHeight)
	DirectX::XMFLOAT3 mTerrainScale;	// scaling

	CComPtr<ID3D11Buffer> mpVertexBuffer;
	CComPtr<ID3D11Buffer> mpIndexBuffer;
	unsigned mNumIndex;

private:
	Terrain(const Terrain&);	/// NO COPYING
	void operator=(const Terrain&);	/// NO NO COPYING

};	// class

}	// namespace