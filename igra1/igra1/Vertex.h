/*==============================================
 * IGRA Vertex structures
 *
 * Written by <YOUR NAME HERE>
 *==============================================*/
#pragma once

#include "DxCommon.h"

namespace igra
{
/** The Vertexes hold all the various Vertex Layouts used in an IGRA application.

As the semester progresses this class will expand, adding more vertex in.

YOU MUST NOT remove/change the existing Vertexes as this will break back compatibity with the earlier examples.

Tip:
Provided the structre does not contain any padding, you can simplify the layouts using the D3D11_APPEND_ALIGNED_ELEMENT value
Alternatively try using the offsetof() macro to get the correct layout

\code
const D3D11_INPUT_ELEMENT_DESC TerrainVertex_layout[] =
{
	// the POSITION element is the next element
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	// the TEXCOORD is at the location of TerrainVertex::TexC
	{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(TerrainVertex,TexC), D3D11_INPUT_PER_VERTEX_DATA, 0 },  
}
\endcode
*/

/** The simple coloured vertex class.
Holds position & colour only.
*/
struct ColouredVertex
{
	DirectX::SimpleMath::Vector3 pos;
	DirectX::SimpleMath::Color colour;
};
const D3D11_INPUT_ELEMENT_DESC ColouredVertex_layout[] =
{
    { "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
const UINT ColouredVertex_layout_len = ARRAYSIZE(ColouredVertex_layout);

/** The main vertex class.
Holds position, normals, texture coords & tangents (for normal maps).
*/
struct Vertex
{
	Vertex(){}
	Vertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT2& uv)
		: Position(p), Normal(n), TangentU(t), TexC(uv){}
	Vertex(
		float px, float py, float pz, 
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v)
		: Position(px,py,pz), Normal(nx,ny,nz),
			TangentU(tx, ty, tz), TexC(u,v){}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 TangentU;
	DirectX::XMFLOAT2 TexC;
};

const D3D11_INPUT_ELEMENT_DESC Vertex_layout[] =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
};
const UINT Vertex_layout_len = ARRAYSIZE(Vertex_layout);


/** The TerrainVertex holds only Position & Textures.
It does not hold normals, because it is not lit, it uses pre-generated lightmaps
*/
struct TerrainVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TexC;
};

const D3D11_INPUT_ELEMENT_DESC TerrainVertex_layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
};
const UINT TerrainVertex_layout_len = ARRAYSIZE(TerrainVertex_layout);



}