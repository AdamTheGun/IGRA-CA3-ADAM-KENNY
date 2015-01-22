/*==============================================
 * IGRA Obj Model Loader
 *
 * Written by Mark Gossage & http://braynzarsoft.net/
 *==============================================*/
#pragma once

/** http://en.wikipedia.org/wiki/Wavefront_.obj_file
As DirectX10 threw away the .X format
and did not provide any suitable replacement
developers have to write their own.

The obj format provides basic support for static models
(with or without texture). Which the model loader is capable of reading.

Original code is based upon: http://limegarden.net/2010/03/02/wavefront-obj-mesh-loader/
which is a simple loader which was not designed for any graphics library.
Subsiquently modified to include a material loader and DX code
(using the MeshFromObj10 example in the June 2010 DXSDK as a reference)

*/
#include <vector>
#include <string>

namespace igra
{

class ModelLoad
{
public:
	struct Vector2f
	{
	    float x, y;
	};
	struct Vector3f
	{
	    float x, y, z;
	};
	struct Vertex
	{

		Vector3f pos,norm;
		Vector2f tex;
	};
	struct Material
	{
		std::wstring matMame;
		Vector3f ambient,diffuse,specular;
		float alpha,shinyness;
		std::wstring texFilename;
	};

	/// simplest loading function
	/// finds the material file
	static bool SimpleLoadObj(const wchar_t* filename,std::vector<Vertex>& verts,std::wstring& outMtlFile);

	/// simplest material loader
	static bool SimpleLoadMtl(const wchar_t* filename,std::vector<Material>& mats);
};

}	// namespace