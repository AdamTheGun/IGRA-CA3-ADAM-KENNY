/*==============================================
 * Simple 3D wireframe drawing for diagnostics
 *
 * Written by IGRA Team
 *==============================================*/
#pragma once

// igra
#include "DxCommon.h"	// Common Dx stuff
#include "ShaderManager.h"
#include "Vertex.h"
// DXTK
#include "PrimitiveBatch.h"
// DX
#include <DirectXColors.h>


/** Note: this code is mainly for diagnostic purposes.
It can & will have a serious performance hit.

The basic usage is as follows:

\code
// class requires:
std::unique_ptr<PrimitiveBatch<ColouredVertex>> mpDraw3D;
...

// initialisation
mpDraw3D.reset(new PrimitiveBatch<ColouredVertex>(GetContext()));
...

// Prepare & begin
Draw3DPrepare(GetContext(),mpShaderManager.get(),view,proj);
mpDraw3D->Begin();

// draw all the bounds
BoundingBox bb(...);
Draw3DBoundingBox(mpDraw3D.get(),bb);

mpDraw3D->End();

// reset the topology after the Draw3D
GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
\endcode

*/
namespace igra
{

void Draw3DPrepare(ID3D11DeviceContext* pContext,ShaderManager* pShader,DirectX::SimpleMath::Matrix view,DirectX::SimpleMath::Matrix proj);
	
void Draw3DBoundingBox(DirectX::PrimitiveBatch<ColouredVertex>* pBatch,const DirectX::BoundingBox& bbox,const DirectX::XMVECTORF32& color=DirectX::Colors::HotPink);

void Draw3DBoundingSphere(DirectX::PrimitiveBatch<ColouredVertex>* pBatch,const DirectX::BoundingSphere& bsph,const DirectX::XMVECTORF32& color=DirectX::Colors::HotPink);

}