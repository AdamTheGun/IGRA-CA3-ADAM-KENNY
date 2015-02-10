/*==============================================
 * GameNodes, MeshNode & CameraNodes for GDEV engine
 *
 * Written by <YOUR NAME>
 *==============================================*/

#include "IgraApp.h"
#include "Node.h"
#include "ModelObj.h"

using DirectX::SimpleMath::Matrix;

namespace igra
{

Node::Node(const Vector3& pos,const Vector3& hpr)
:mPos(pos),mHpr(hpr)
{}

void Node::Match(const Node& other)
{
	mPos=other.mPos;
	mHpr=other.mHpr;
}

Vector3 Node::RotateVector(const Vector3& vec)
{
	Matrix rot= Matrix::CreateFromYawPitchRoll(mHpr.x,mHpr.y,mHpr.z);
	return Vector3::TransformNormal(vec,rot);
	// create the rotation matrix, then use Vector3::TransformNormal() multiply the vector
}
void Node::Move(const Vector3& vec)
{
	Vector3 rvec=RotateVector(vec);	// rotated vec
	mPos+=rvec;
}

void Node::MoveGround(const Vector3& delta)
{
	Vector3 rvec=RotateVector(delta);	// rotated vec
	rvec.y=0;	// no y movement
	mPos+=rvec;
}

void Node::LookAt(const Vector3& tgt)
{
	SetLookDirection(tgt-mPos);
}

void Node::LookAt(const Node& tgt)
{
	SetLookDirection(tgt.mPos-mPos);
}

void Node::SetLookDirection(const Vector3& dir)
{
	float len = dir.Length();
	if(len<0.0000000001f)return;
	mHpr.x=atan2f(dir.x,dir.z);
	mHpr.y=-(asinf(dir.y/len));
	mHpr.z = 0;
	// trigonometry, you still need it!
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////


void DrawableNode::Init(ModelObj* pModel,const Vector3& pos)
{
	mpModel=pModel;
	mPos=pos;
}

DirectX::SimpleMath::Matrix DrawableNode::GetWorldMatrix()
{
	Matrix trans= Matrix::CreateTranslation(mPos);
	Matrix rot= Matrix::CreateFromYawPitchRoll(mHpr.x,mHpr.y,mHpr.z);
	Matrix scale= Matrix::CreateScale(mScale);
	
	/* world = scale rotate trans */
	return scale*rot*trans;
}

void DrawableNode::Draw(ID3D11DeviceContext* pContext,const Matrix& view,const Matrix& projection)
{
	Matrix world = GetWorldMatrix();
	mpModel->mMaterial.FillMatrixes(world,view,projection);
	
	mpModel->mMaterial.Apply(pContext);
	mpModel->Draw(pContext);
	// GetWorldMatrix(), FillMatrixes(), Apply(), Draw()
}

DirectX::BoundingSphere DrawableNode::GetBounds(float scaleby)
{
	/*DirectX::BoundingSphere bs;
	mpModel->GetBounds().Transform(bs,GetWorldMatrix());
	bs.Radius*=scaleby;
	return bs;*/
	float radius=mpModel->GetBounds().Radius;
	return DirectX::BoundingSphere(mPos,radius*mScale*scaleby);
}

DirectX::BoundingBox DrawableNode::GetBox()
{
	Vector3 size = Vector3(mpModel->GetBBox().Extents.x, mpModel->GetBBox().Extents.y, mpModel->GetBBox().Extents.z);
	return DirectX::BoundingBox(mPos, size * mScale * 0.65f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

CameraNode::CameraNode(const Vector3& pos,const Vector3& turn,
			float fov, float aspect,float nearDist,float farDist)
{
	Init(pos,turn,fov,aspect,nearDist,farDist);
}
void CameraNode::Init(const Vector3& pos,const Vector3& turn,
			float fov, float aspect,float nearDist,float farDist)
{
	mPos=pos;
	mHpr=turn;
	mFov=fov;
	mAspect=aspect;
	mNear=nearDist;
	mFar=farDist;
}

void CameraNode::SetLens(float fov,float aspect,float nearDist,float farDist)
{
	mFov=fov;
	mAspect=aspect;
	mNear=nearDist;
	mFar=farDist;
}

void CameraNode::SetFov(float fov)
{
	mFov=fov;
}
void CameraNode::SetAspect(float aspect)
{
	mAspect=aspect;
}
void CameraNode::SetNearFar(float nearDist,float farDist)
{
	mNear=nearDist;
	mFar=farDist;
}

Matrix CameraNode::GetProjectionMatrix()
{
	
	// XMMatrixPerspectiveFovLH
	// using LH rather than RH as all the DX books are LH based

	return DirectX::XMMatrixPerspectiveFovLH(mFov,mAspect,mNear,mFar);
}

Matrix CameraNode::GetViewMatrix()
{
	
	// easy way: using the XMMatrixLookToLH() fn
	// we just need to get the rotation matrix & transform the up & forward Vector3's
	// using LH rather than RH as all the DX books are LH based
	Matrix rot= Matrix::CreateFromYawPitchRoll(mHpr.x,mHpr.y,mHpr.z);
	Vector3 forward =Vector3::TransformNormal(Vector3(0,0,1),rot);
	Vector3 up =Vector3::TransformNormal(Vector3(0,1,0),rot);
	
	
	return DirectX::XMMatrixLookToLH(mPos,forward,up);

}
////////////////////////////////////////////////////////////////////////////////////////////////////



}	// namespace