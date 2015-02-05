/*==============================================
 * ModelObj for IGRA
 *
 * Written by <YOUR NAME>
 *==============================================*/


#include "ModelObj.h"	// the main class
#include "ModelLoad.h"	// the loader
#include "IgraUtils.h"	// igra
#include "IgraApp.h"	// igra
#include "Vertex.h"	// igra

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace igra
{
ModelObj::ModelObj()
{
	mNumIndex=0;
}


void ModelObj::Load(ID3D11Device* pDev, ShaderManager* pShaderManager, const wchar_t* filename_obj,bool enableLights)
{
	std::vector<igra::Vertex> verts;
	std::vector<UINT> indexs;
	std::wstring mtlFilename;
	// load geometry
	LoadGeometry(filename_obj,mtlFilename,verts,indexs);
	// create buffers
	CreateGeometry(pDev,verts,indexs);
	// compute bounding box/sphere
	ComputeBounds(verts);
	// load materials:
	std::wstring path=ExtractPath(filename_obj);
	LoadMaterial(pDev, pShaderManager,enableLights,path,mtlFilename);

}

void ModelObj::LoadGeometry(const wchar_t* filename_obj,std::wstring& outMtlFilename,std::vector<igra::Vertex>& outVerts,std::vector<UINT>&outIndexs)
{
	// load it
	std::vector<ModelLoad::Vertex> objverts;
	if (!ModelLoad::SimpleLoadObj(filename_obj,objverts,outMtlFilename))
		FAIL(filename_obj);

	// now convert from ModelLoad::Vertex to igra::Vertex
	outVerts.resize(objverts.size());	// grow to correct size
	outIndexs.resize(objverts.size());	// ditto
	for(unsigned i=0;i<objverts.size();++i)
	{
		outVerts[i].Position=XMFLOAT3(objverts[i].pos.x,
                                    objverts[i].pos.y,objverts[i].pos.z);
		outVerts[i].Normal=XMFLOAT3(objverts[i].norm.x,
                                    objverts[i].norm.y,objverts[i].norm.z);
		outVerts[i].TexC=XMFLOAT2(objverts[i].tex.x,objverts[i].tex.y);
		outVerts[i].TangentU=Vector3(1,0,0);	// dummy value
		outIndexs[i]=i;
	}

}
void ModelObj::ComputeTangents(std::vector<igra::Vertex>& verts,const std::vector<UINT>& indexs)
{
	// Lengyel, Eric. “Computing Tangent Space Basis Vectors for an Arbitrary Mesh”. 
	// Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html
	int triangleCount=indexs.size()/3,vertexCount=verts.size();
	std::vector<Vector3> tan1,tan2;
	tan1.resize(vertexCount,Vector3(0,0,0));
	tan2.resize(vertexCount,Vector3(0,0,0));

	for (int a = 0; a < triangleCount; a++)
	{
		long i1 = indexs[a*3+0];
		long i2 = indexs[a*3+1];
		long i3 = indexs[a*3+2];
        
		const XMFLOAT3& v1 = verts[i1].Position;
        const XMFLOAT3& v2 = verts[i2].Position;
        const XMFLOAT3& v3 = verts[i3].Position;
        
        const XMFLOAT2& w1 = verts[i1].TexC;
        const XMFLOAT2& w2 = verts[i2].TexC;
        const XMFLOAT2& w3 = verts[i3].TexC;

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;
        
        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;
        
        float r = 1.0f/(s1 * t2 - s2 * t1);
        Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                (t2 * z1 - t1 * z2) * r);
        Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                (s1 * z2 - s2 * z1) * r);
        
        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;
        
        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
	}
    for (long a = 0; a < vertexCount; a++)
    {
        Vector3 n = Vector3(verts[a].Normal.x,verts[a].Normal.y,verts[a].Normal.z);
        Vector3 t = tan1[a];
        
        // Gram-Schmidt orthogonalize
		Vector3 tang=(t - n * n.Dot(t));
		tang.Normalize();
        verts[a].TangentU = tang;
        
        // Calculate handedness
        //tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
    }
}

void ModelObj::CreateGeometry(ID3D11Device* pDev, const std::vector<igra::Vertex>& verts,const std::vector<UINT>& indexs)
{
	mpVertexBuffer=CreateVertexBuffer(pDev,&verts[0],sizeof(igra::Vertex) * verts.size());
	mpIndexBuffer=CreateIndexBuffer(pDev,&indexs[0],indexs.size());
	mNumIndex=indexs.size();
}

void ModelObj::LoadMaterial(ID3D11Device* pDev, ShaderManager* pShaderManager,bool enableLights,const std::wstring& directory,const std::wstring& filename_mtl)
{
	if (filename_mtl.length()<1)
	{
		// no material file: so setup a default:
		mMaterial.mpShaderManager=pShaderManager;
		mMaterial.mpLayout=pShaderManager->LayoutDefault();
		mMaterial.mpVS=pShaderManager->VSDefault();
		mMaterial.mpPS=pShaderManager->PSUnlit();
		mMaterial.mMaterial.gMaterial.Ambient=
                       mMaterial.mMaterial.gMaterial.Diffuse=Color(1,0,1,1);
		mMaterial.mMaterial.gMaterial.Specular=
                       mMaterial.mMaterial.gMaterial.Reflect=Color(0,0,0,1);
		mMaterial.mMaterial.gUseTexture=false;
		return;
	}
	// load material:
	std::vector<ModelLoad::Material> objmats;
	std::wstring mtlFilename=CombinePath(directory,filename_mtl);
	if (!ModelLoad::SimpleLoadMtl(mtlFilename.c_str(),objmats))
		FAIL(mtlFilename.c_str());

	// convert material:
	ModelLoad::Material m=objmats[0];	// just use the first one

// TODO:
// set mMaterial.mMaterial.gMaterial based upon the value of m
// you will need to set Ambient, Diffuse and Specular
// also need to set Diffuse.w=m.alpha to permit transparency (coming later)
// if m.shinyness>0 then set Specular.w=m.shinyness which is the shininess
// keep Reflect=(0,0,0,0)

	mMaterial.mMaterial.gMaterial.Diffuse.w = m.alpha;
	mMaterial.mMaterial.gMaterial.Diffuse = Color(m.diffuse.x,m.diffuse.y,m.diffuse.z);
	mMaterial.mMaterial.gMaterial.Ambient.w = (m.ambient.x,m.ambient.y,m.ambient.z);
	mMaterial.mMaterial.gMaterial.Reflect = Color(0,0,0,0);
	mMaterial.mMaterial.gMaterial.Specular = Color(m.specular.x,m.specular.y,m.specular.z);
	if(m.shinyness>0)
	{
		mMaterial.mMaterial.gMaterial.Specular.w = m.shinyness;
	}
	else
	{
		//mMaterial.mMaterial.gMaterial.Specular=mMaterial.mMaterial.gMaterial.Reflect=Color(0,0,0,0);
	}
		


	// just load the texture
	if (m.texFilename.length()>0)	// there is a texture
	{
// TODO:
// load the texture & set mMaterial.mpTexture to it

		mMaterial.mMaterial.gUseTexture=true;
		std::wstring fName = CombinePath(directory,m.texFilename);
		mMaterial.mpTexture.Attach(CreateTextureResourceWIC(pDev,fName.c_str()));
	}
	else
	{
			mMaterial.mMaterial.gUseTexture=false;
	}

	// sort out the shaders
// TODO:
// set mMaterial.mpShaderManager, mpLayout, mpVS, mpPS
// and mMaterial.mMaterial.gUseTexture 
// you will need to look at the enableLights variable & if you loaded a texture
	mMaterial.mpShaderManager = pShaderManager;
	mMaterial.SetupDefaultLights();
	
	mMaterial.mpLayout=pShaderManager->LayoutDefault();
	mMaterial.mpVS=pShaderManager->VSDefault();
	mMaterial.mpPS=pShaderManager->PSDirLight();
	
}

void ModelObj::ComputeBounds(const std::vector<igra::Vertex>& verts)
{
	BoundingBox::CreateFromPoints(mBBox,verts.size(),
                                     &verts[0].Position, sizeof(igra::Vertex));
	BoundingSphere::CreateFromPoints(mBSphere,verts.size(),
                                     &verts[0].Position, sizeof(igra::Vertex));
}

void ModelObj::Draw(ID3D11DeviceContext* pContext)
{
		// Set the vertex/index buffer
	ShaderManager::SetVertexIndexBuffers(pContext,mpVertexBuffer,sizeof(Vertex),mpIndexBuffer);
	// do the actual draw
	pContext->DrawIndexed(mNumIndex,0,0);	// number of indexes, not shapes
}


}