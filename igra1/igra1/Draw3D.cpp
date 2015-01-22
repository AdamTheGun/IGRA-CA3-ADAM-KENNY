/*==============================================
 * Simple 3D wireframe drawing for diagnostics
 *
 * Written by IGRA Team
 *==============================================*/
#pragma once

#include "Draw3D.h"	// header
using namespace DirectX::SimpleMath;

namespace igra
{

void Draw3DPrepare(ID3D11DeviceContext* pContext,ShaderManager* pShader,DirectX::SimpleMath::Matrix view,DirectX::SimpleMath::Matrix proj)
{
	// set usual VS,PS,IA
	pContext->VSSetShader(pShader->VSColoured(), 0, 0);
	pContext->PSSetShader(pShader->PSColoured(), 0, 0);
	pContext->IASetInputLayout(pShader->LayoutColoured());

	// usual setting of matrix const buffer
	ShaderManager::cbMatrixBasic cBuff;
	cBuff.gWorldViewProj=(view*proj).Transpose();	// note: world will be applied directly to the bounds, so world is identity
	ShaderManager::SetVSConstantBuffer(pContext,pShader->CBMatrixBasic(),&cBuff);
}
	
void Draw3DBoundingBox(DirectX::PrimitiveBatch<ColouredVertex>* pBatch,const DirectX::BoundingBox& bbox,const DirectX::XMVECTORF32& color)
{
	Vector3 bbcorners[8];
	bbox.GetCorners(bbcorners);
	ColouredVertex corners[8];
	Color col(color.f);
	for(unsigned i=0;i<8;i++)
	{
		corners[i].pos=bbcorners[i];
		corners[i].colour=col;
	}
	uint16_t edges[]={0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7};
	pBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST,edges,24,corners,8);
}

void Draw3DBoundingSphere(DirectX::PrimitiveBatch<ColouredVertex>* pBatch,const DirectX::BoundingSphere& bsph,const DirectX::XMVECTORF32& color)
{
	Color col(color.f);
	const int STEPS=32;
	ColouredVertex points[STEPS*3];	// 3 rings:
	float cx=bsph.Center.x,cy=bsph.Center.y,cz=bsph.Center.z,r=bsph.Radius;
	for(int i=0;i<STEPS;i++)
	{
		float ang=i*DirectX::XM_2PI/STEPS;
		// first ring is XY axis
		points[i].pos=Vector3(cx+r*sinf(ang),cy,cz+r*cosf(ang));
		// second ring is XZ axis
		points[STEPS+i].pos=Vector3(cx+r*sinf(ang),cy+r*cosf(ang),cz);
		// third is YZ
		points[2*STEPS+i].pos=Vector3(cx,cy+r*sinf(ang),cz+r*cosf(ang));
	}
	// add colour to all
	for(int i=0;i<STEPS*3;i++)
		points[i].colour=col;

	uint16_t index[STEPS*3*2];	// 3 rings, 2 indexes per line
	for(int i=0;i<STEPS;i++)
	{
		index[i*2]=i;
		index[i*2+1]=(i+1)%STEPS;
		index[2*STEPS+i*2]=STEPS+i;
		index[2*STEPS+i*2+1]=STEPS+(i+1)%STEPS;
		index[4*STEPS+i*2]=2*STEPS+i;
		index[4*STEPS+i*2+1]=2*STEPS+(i+1)%STEPS;
	}
	pBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST,index,STEPS*6,points,STEPS*3);
}


}