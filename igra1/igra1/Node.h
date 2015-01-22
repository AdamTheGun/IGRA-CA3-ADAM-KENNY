/*==============================================
 * GameNodes, MeshNode & CameraNodes for IGRA
 *
 * Written by <YOUR NAME>
 *==============================================*/
#pragma once

#include "DxCommon.h"	// Common Dx stuff
#include "Draw3D.h"	// diagnostic stuff

namespace igra
{
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Matrix;
/// pre decl
class ModelObj;


/** The Node class is the basic (position & orientation) class.
It provides basic movement capabilities & little else.
Its derived classes are the main things to use:

However if you write code to manipulate the Node class,
it will work equally well for Mesh's & Camera's.
This is one of the key reasons for this base class.
*/
class Node
{
public:	// data is public, though not often used
	Vector3 mPos;	///< position
	Vector3 mHpr;	///< turning (yaw/heading, pitch, roll in radians)
public:

	Node(const Vector3& pos=Vector3(0,0,0),const Vector3& hpr=Vector3(0,0,0));

	// accessor
	const Vector3& GetPos(){return mPos;}
	void SetPos(const Vector3& pos){mPos=pos;}
	void SetPos(float x, float y, float z){mPos.x=x; mPos.y=y; mPos.z=z;}
	/** orientation: Yaw, Pitch & Roll in Radians.
	Because the angles are stored as 3 euler angles, it is not
	possible to perform airplane (cockpit style) manuvers.
	\par eg. 
	if you roll 90 degrees counter clockwise, then pitch up
	in an aircraft you would be turning left. However in the node's case
	if you roll 90 degrees counter clockwise, then pitch up, you just pitch up.
	*/
	const Vector3& GetHpr(){return mHpr;}
	void SetHpr(const Vector3& hpr){mHpr=hpr;}
	void SetHpr(float h, float p, float r){mHpr.x=h; mHpr.y=p; mHpr.z=r;}

	/// Sets this nodes position & orientation to match the other
	void Match(const Node& other);

	/** returns a vector rotated by the Nodes orientation.
	This is very useful for determining directions & similar.
	\param vec the vector to be rotated
	\returns the rotated vector
	EG.
	\code
	Node node;
	...
	// to determine what is 'forward' direction WRT the node.
	Vector3 forward=node.RotateVector(Vector3(0,0,1));
	// to get a suitable position which is behind the node
	Vector3 behindNode=node.RotateVector(Vector3(0,10,-20))+
							node.GetPos();
	\endcode
	\see OffsetPos to get the direction+ the current position
	*/
	Vector3 RotateVector(const Vector3& vec);

	/// \defgroup NodeMove Movement code for Node,CMeshNode & CCamera
	/// @{
	/** Moves the node by a certain amount in its current direction.
	If you want to have simple movement of the object, without considering orientation.
	You can just use the Position() function.
	\code
	// this code moves the object but does not consider orientation
	Vector3 delta(0,0,0);
	if (KeyDown(VK_LEFT))
		delta.x+=1.0f
	if (KeyDown(VK_RIGHT))
		delta.x-=1.0f
	// etc,etc
	node.SetPos(node.GetPos()+delta*SPEED);
	\endcode
	However the above code does not consider orientation of the node.
	The Move() code does consider orientation.
	\code
	// this code moves the object considering orientation
	Vector3 dir(0,0,0);
	if (KeyDown('A'))
		dir.x-=1.0f
	if (KeyDown('D'))
		dir.x+=1.0f
	// etc,etc
	node.Move(dir*SPEED); // move it accordingly
	\endcode

	\note For the coordinates, it assumes forwards is +Z & right is +X (in DX, not in 3DSMax)
	(so if your models are not facing that way, it might look strange).
	\todo get someone who uses max to find out which way the coordinates look
	*/
	void Move(const Vector3& delta);
	/// Special version of Move() which does not change the Y axis.
	/// \see Move() for full details
	void MoveGround(const Vector3& delta);
	/// Yaws the by the amount of radians, this is turning right
	void Yaw(float amount){mHpr.x+=amount;}
	/// Pitches the by the amount of radians, this is turning down
	void Pitch(float amount){mHpr.y+=amount;}
	/// Rolls the by the amount of radians, this is counter clockwise (left down,right up)
	void Roll(float amount){mHpr.z+=amount;}
	/// Turns (yaw,pitch,roll) the the specfied amount
	void Turn(const Vector3& turn){mHpr+=turn;}

	/// @}
	/** makes the node look at the a point in space.
	This code sets the pitch & yaw of the node, and sets roll=0
	\param tgt the POSITION to look at
	\note will do nothing if the target point is at the nodes location
	(you cannot look at yourself)
	\see SetLookDirection for information on setting the DIRECTION to look
	*/
	void LookAt(const Vector3& tgt);
	/** makes the node look at another node (or mesh-node)
	\param tgt the Node to look at
	Eg.
	\code
	Node target;
	Node seeker;
	...
	seeker.LookAt(target); // seeker will now face the target
	\endcode
	This code sets the pitch & yaw of the node, and sets roll=0
	\note will do nothing if the target point is at the nodes location
	(you cannot look at yourself)
	\see SetLookDirection for information on setting the DIRECTION to look
	*/
	void LookAt(const Node& tgt);
	/** makes the node look in a given direction.
	Only sets the Yaw and Pitch values, sets Roll to Zero.
	\param dir the DIRECTION to look in (x axis, y axis or otherwise)
		This need not be normalised, but is should not be (0,0,0).
		If it is zero, then the code will ignore the request (its meaningless to look nowhere)
	\code
	Node node;
	// look in the Z direction (forward)
	node.SetLookDirection(Vector3(0,0,1));
	// look in the X direction (right)
	node.SetLookDirection(Vector3(1,0,0));
	// look in the Y direction (up)
	node.SetLookDirection(Vector3(0,1,0));
	\endcode
	\see LookAt for information on looking AT a position 
	*/
	void SetLookDirection(const Vector3& dir);
private:
	Node(const Node&); // no copying
	void operator=(const Node&); // no copying
};

/** DrawableNode is the basic actor class with a mesh & life.
It is designed to be derived from and to have its Update() function
overridden.
*/
class DrawableNode: public Node
{
public:
	DrawableNode()
		:mpModel(nullptr),mScale(1),mHealth(100)
	{
	}
	virtual ~DrawableNode(){}
    /** Initalise the object.
	\param pModel the model to use (will not be delete'd at destruction)
	\param pos the position on the object
	*/
	void Init(ModelObj* pModel,const Vector3& pos=Vector3(0,0,0));

	/// Gets the nodes world matrix.
	/// this takes a bit of time to compute, so try not to call it needlessly.
	Matrix GetWorldMatrix();

    /** updates/moves the object
    virtual function which can be overridden for derived classes
	\note This function should not normally be called on a dead object.
	  But it does not check to see if the object is alive (caller is expected to do that)
	*/
	virtual void Update(){}
    /** draws the object.
	\note This function should not normally be called on a dead object.
	  But it does not check to see if the object is alive (caller is expected to do that)
	*/
	void Draw(ID3D11DeviceContext* pContext,const Matrix& view,const Matrix& projection);

	/// uniform scale of the object.
	/// (non uniform scaling is not permitted as it make collision more complex)
	float mScale;

	/// the model, should be set using Init()
	ModelObj* mpModel;

	/// health of object
	/// Once the object has <1 life is considered 'dead' and caller should not draw/update
	int mHealth;
	/// returns true if the object is alive
	bool IsAlive(){return mHealth>0;}
	/// kills the object (sets life=0)
	void Kill(){mHealth=0;}

	/** Returns the BoundingSphere of the object.
	This is based on the node's location, scale and the models scale.
	As this is usually a little large the scaleby factor is added to reduce the size */
	DirectX::BoundingSphere GetBounds(float scaleby=0.75f);
};
//////////////////////////////////////////////////////////////////////////

/** The CameraNode is a node for holding camera infomation.
Like all nodes it can be moved about & made to look at other nodes.
It also has all the code to setup view & projection matrixes.

This class is good for First & Third person views as well as for overhead & fixed viewpoints.
Like the Node class: its not really suitable for Cockpit views.

As well as the Camera position & orientation, it also has near & far planes as well as 
field of view and aspect ratio.

The best thing about this class is that being derived from the Node class, 
anything you could be with a Node (or MeshNode), you can also do with a CameraNode.

Eg. Its one line of code to do a first person camera
\code
camera.Match(player); // camera has the same position & orientation as player
\endcode
And its not to hard to do other camera nodes.
*/
class CameraNode: public Node
{
protected:
	float mFov;	///< field of view in radians
	float mAspect;	///< aspect ratio
	float mNear,mFar;	///< near & far clipping
public:
	/// Constuctor: see Init for the parameters
	CameraNode(const Vector3& pos=Vector3(0,0,0),
				const Vector3& turn=Vector3(0,0,0),
				float fov=DirectX::XM_PI/4, float aspect=1.333f,float nearDist=1.f,float farDist=1000.f);
	/** Sets camera parameters.
	\param pos,turn as per the node
	\param fov field of view in radians: PI/4 (45 degrees) is the normal
	\param aspect aspect ratio: for PC's is usually 4/3 (1.333) or 16/9 (1.777)
	\param near,far near & far clipping distance
	*/
	void Init(const Vector3& pos=Vector3(0,0,0),
				const Vector3& turn=Vector3(0,0,0),
				float fov=DirectX::XM_PI/4, float aspect=1.333f,float nearDist=1.f,float farDist=1000.f);

	float GetFov(){return mFov;}	///< gets the field of view
	float GetAspect(){return mAspect;}	///< gets the aspect ratio
	float GetNear(){return mNear;}	///< gets the near clip
	float GetFar(){return mFar;}	///< gets the far clip
	/// Sets camera parameters.
	/// \see Init() for parameters
	void SetLens(float fov,float aspect,float nearDist,float farDist);
	void SetFov(float fov);	///< sets field of view
	void SetAspect(float aspect);	///< sets aspect ratio
	void SetNearFar(float nearDist,float farDist);	///< sets near & far clip distance

	/// Gets the view matrix.
	/// this takes a bit of time to compute, so try not to call it needlessly.
	Matrix GetViewMatrix();
	/// Gets the projection matrix.
	/// this takes a bit of time to compute, so try not to call it needlessly.
	Matrix GetProjectionMatrix();
};
////////////////////////////////////////////////////////////////////////////////////////


/** \defgroup MeshNodeFn Mesh node functions.
One of the most common ideas in computer games is the idea of groups of objects.
For example:
\code
// THIS IS NOT REAL C++ CODE, DO NOT COPY
ObjectGroup Enemies;
ObjectGroup Bullets;
Object Player;
...
// move player about
...
if (KeyPress(SPACE))
	Bullets.Add(new Bullet(....));
Enemies.MoveAll(); // moves all the enemies
Bullets.MoveAll(); // moves all the bullets
// add the collision code 
...
\endcode

For the IGRA nodes, the most common use is an object pool:
\ref http://gameprogrammingpatterns.com/object-pool.html
Simply put, its a std::vector<DrawableNode*> where each DrawableNode may of may not be active at the time
(you need to check the IsAlive() attribute)

If you need a new object, hunt through the vector & find a non-alive one & reuse it.

When you update/draw, just go through the vector & update/draw all the ones which are alive.

All you need to do, is remember to fill the vector with objects at the start & delete them all at the end.

The code was designed to use vectors of Object pointers, which allows polymorphism, but also is a performance hit 
(see \ref http://gameprogrammingpatterns.com/data-locality.html for why). It could be optimised by not using the pointers,
but that would reduce flexibility.

Its also using templates because its more useful that way

@{
*/

/** Updated/Moves all living members of the group.
This code can be though of as: (foreach living object: call Update)
\param nodes the vector of DrawableNode
*/
template<class T>
void UpdateAliveNodes(const std::vector<T*>& nodes)
{
	//FAIL("TODO: UpdateAliveNodes()");
	// simple for loop, calling Update() on all nodes which IsAlive() is true
	for(unsigned i=0;i<nodes.size();++i)
	{
		if (nodes[i]->IsAlive())
			nodes[i]->Update();
	}
}

/** Draws all living members of the group.
This code can be though of as: (foreach living object: call Draw)
\param nodes the vector of DrawableNode
\param pContext the DX device context
\param view,projection the two matrixes
*/
template<class T>
void DrawAliveNodes(const std::vector<T*>& nodes,ID3D11DeviceContext* pContext,const Matrix& view,const Matrix& projection)
{
	//FAIL("TODO: DrawAliveNodes()");
	// simple for loop, calling Draw() on all nodes which IsAlive() is true
	for(unsigned i=0;i<nodes.size();++i)
	{
		if (nodes[i]->IsAlive())
			nodes[i]->Draw(pContext,view,projection);
	}
}

/** Draws bounds of all living members of the group.
This code can be though of as:
\param nodes the vector of DrawableNode
*/
template<class T>
void DrawAliveNodeBounds(const std::vector<T*>& nodes,DirectX::PrimitiveBatch<ColouredVertex>* pBatch,const DirectX::XMVECTORF32& color=DirectX::Colors::HotPink)
{
	// simple for loop, calling Draw3DBoundingSphere() on all nodes which IsAlive() is true
	for(unsigned i=0;i<nodes.size();++i)
	{
		if (nodes[i]->IsAlive())
			Draw3DBoundingSphere(pBatch,nodes[i]->GetBounds(),color);		
	}
}

/** Searches through the vector looking for a 'dead' item & then returns it.
\returns a 'dead' item or a nullptr.
\warn you must check for null, otherwise your code might crash.
\code
std::vector<DrawableNode*> pShots;
// BAD CODE
DrawableNode* ptr=FindDeadNode(pShots);
ptr->mLife=100;	// this will CRASH is ptr is nullptr
...
// GOOD CODE
DrawableNode* ptr=FindDeadNode(pShots);
if (ptr) // check to make sure that ptr is not nullptr
{
  ptr->mLife=100;	// guaranteed not to be null
  ...
}
\endcode
*/
template<class T>
T* FindDeadNode(const std::vector<T*>& nodes)
{
	for(unsigned i=0;i<nodes.size();++i)
	{
		if (!nodes[i]->IsAlive())
			return nodes[i];
	}
	return nullptr;
}



/** Deletes every member of the vector & then clears the vector.
This code can be though of as: (foreach object: call delete obj; vector.clear())
\param nodes the vector of DrawableNode
*/
template<class T>
void DeleteAllNodes(std::vector<T*>& nodes)
{
	for(unsigned i=0;i<nodes.size();++i)
	{
		delete nodes[i];
	}
	nodes.clear();
}

///@}


};	// namespace