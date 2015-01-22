/*==============================================
 * Model for IGRA
 *
 * Written by <YOUR NAME>
 *==============================================*/
#pragma once

#include "DxCommon.h"	// Common Dx stuff
#include "ShaderManager.h"		// shader stuff needed
#include "Vertex.h"		// vertex declaration

namespace igra
{

/** Wavefront OBJ mode.
Is called this as DirectXTK also has a model class. 
But it does not easily permit control of shaders, so we use this instead.
*/
class ModelObj
{
public:
	ModelObj();
	/** All in one load function, loads a wavefront OBJ file and MTL material.
	Loads the model, generates the geometry, loads the materials & a texture (if applicable)
	*/
	void Load(ID3D11Device* pDev,ShaderManager* pShaderManager,const wchar_t* filename_obj, bool enableLights=true);

	/** sets VB,IB & draws the object
	does not touch the shaders or the BasicMaterial, the caller must sort those out first.
	*/
	void Draw(ID3D11DeviceContext* pContext);

	/// gets bounding sphere
	const DirectX::BoundingSphere& GetBounds(){return mBSphere;}
	/// gets bounding box
	const DirectX::BoundingBox& GetBBox(){return mBBox;}

	/// the models material
	/// left public to allow easy editing if required.
	BasicMaterial mMaterial;

	ID3D11Buffer* GetVertexBuffer(){return mpVertexBuffer;}
	ID3D11Buffer* GetIndexBuffer(){return mpIndexBuffer;}
	unsigned GetNumberIndexes(){return mNumIndex;}
private:
	/// Loads the geometry & gets material filename
	void LoadGeometry(const wchar_t* filename_obj,std::wstring& outMtlFilename,std::vector<igra::Vertex>& outVerts,std::vector<UINT>& outIndexs);
	/// Loads the geometry & gets material filename
	void ComputeTangents(std::vector<igra::Vertex>& verts,const std::vector<UINT>& indexs);
	/// creates the geometry
	void CreateGeometry(ID3D11Device* pDev, const std::vector<igra::Vertex>& verts,const std::vector<UINT>& indexs);
	/// fills the BasicMaterial parameters & shaders, loads the texture (if needed)
	void LoadMaterial(ID3D11Device* pDev, ShaderManager* pShaderManager,bool enableLights,const std::wstring& directory,const std::wstring& filename_mtl);
	/// compute bounding box & sphere
	void ComputeBounds(const std::vector<igra::Vertex>& verts);
private:
	CComPtr<ID3D11Buffer> mpVertexBuffer;
	CComPtr<ID3D11Buffer> mpIndexBuffer;
	unsigned mNumIndex;

private:
	DirectX::BoundingBox mBBox;
	DirectX::BoundingSphere mBSphere;
private:
	ModelObj(const ModelObj&);	/// NO COPYING
	void operator=(const ModelObj&);	/// NO NO COPYING
};

}
