#include <fstream>
#include <sstream>
#include "ModelLoad.h"

namespace igra
{

/* Internal structure */
struct _ObjMeshFaceIndex{
    int pos_index[3];
    int tex_index[3];
    int nor_index[3];
	int material;
};

bool RawParseObj(const std::wstring& filename,std::vector<ModelLoad::Vector3f>& positions,std::vector<ModelLoad::Vector2f>& texcoords,std::vector<ModelLoad::Vector3f>& normals,std::vector<_ObjMeshFaceIndex>& faces,std::wstring& outMtlFile);

bool ModelLoad::SimpleLoadObj(const wchar_t* filename,std::vector<Vertex>& verts,std::wstring& outMtlFile)
{
    std::vector<ModelLoad::Vector3f> positions;
    std::vector<ModelLoad::Vector2f> texcoords;
    std::vector<ModelLoad::Vector3f> normals;

    std::vector<_ObjMeshFaceIndex>  faces;

	// reserve space as we are expecting quite a few items
	positions.reserve(5000);
	texcoords.reserve(5000);
	normals.reserve(5000);
	faces.reserve(5000);
	if (!RawParseObj(filename,positions,texcoords,normals,faces,outMtlFile)) return false;

	// convert all the faces into vertex/index information
	//std::vector<UINT> attributes;
	const ModelLoad::Vector3f V3ZERO={0,0,0};
	const ModelLoad::Vector2f V2ZERO={0,0};
    for(unsigned i = 0; i < faces.size(); ++i){
		Vertex v={V3ZERO,V3ZERO,V2ZERO};

		for( int j=0;j<3;j++)
		{
			int posIdx=faces[i].pos_index[j], normIdx=faces[i].nor_index[j], texIdx=faces[i].tex_index[j];

			v.pos=positions[posIdx];
			if (normIdx>=0)
				v.norm=normals[normIdx];
			if (texIdx>=0)
				v.tex=texcoords[texIdx];
			verts.push_back(v);
			//indexes.push_back(verts.size()-1);
		}
		//attributes.push_back(faces[i].material);
    }
	return true;	// all ok
}

bool RawParseObj(const std::wstring& filename,std::vector<ModelLoad::Vector3f>& positions,std::vector<ModelLoad::Vector2f>& texcoords,std::vector<ModelLoad::Vector3f>& normals,std::vector<_ObjMeshFaceIndex>& faces,std::wstring& outMtlFile)
{
	// the load:
	// based upon: http://limegarden.net/2010/03/02/wavefront-obj-mesh-loader/
	const wchar_t* TOKEN_VERTEX_POS= L"v";
	const wchar_t* TOKEN_VERTEX_NOR= L"vn";
	const wchar_t* TOKEN_VERTEX_TEX= L"vt";
	const wchar_t* TOKEN_FACE= L"f";
	const wchar_t* TOKEN_MATERIAL = L"mtllib";
	const wchar_t* TOKEN_USE_MATERIAL = L"usemtl";
    /**
     * Load file, parse it
     * Lines beginning with:
     * '#'  are comments can be ignored
     * 'v'  are vertices positions (3 floats that can be positive or negative)
     * 'vt' are vertices texcoords (2 floats that can be positive or negative)
     * 'vn' are vertices normals   (3 floats that can be positive or negative)
     * 'f'  are faces, 3 values that contain 3 values which are separated by / and <space>
     */

    //std::wifstream filestream;
    //filestream.open(filename);
	//if (!filestream)	return false;	// error
	FILE* file=nullptr;
	_wfopen_s(&file,filename.c_str(),L"r");
	if (file==nullptr)	return false;

	wchar_t line_stream[1024];


	//while(std::getline(filestream, line_stream)){
	while(!feof(file))
	{
		// performance improvement:
		// by changing from std::getline() to fgetws() time taken is ~25% of original!!!
		fgetws(line_stream,1024,file);
		// having a external stringstream & filling it this was seems to make little difference to speed
		std::wstringstream str_stream(line_stream);
		std::wstring type_str;
        str_stream >> type_str;
		//OutputDebugStringA(formatA("=%s= +%s+\n",line_stream,type_str.c_str()));
        if(type_str == TOKEN_MATERIAL){
            str_stream >> outMtlFile;
        }else if(type_str == TOKEN_VERTEX_POS){
            ModelLoad::Vector3f pos;
            str_stream >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }else if(type_str == TOKEN_VERTEX_TEX){
            ModelLoad::Vector2f tex;
            str_stream >> tex.x >> tex.y;
            texcoords.push_back(tex);
        }else if(type_str == TOKEN_VERTEX_NOR){
            ModelLoad::Vector3f nor;
            str_stream >> nor.x >> nor.y >> nor.z;
            normals.push_back(nor);
        }else if(type_str == TOKEN_FACE){
            _ObjMeshFaceIndex face;
            for(int i = 0; i < 3; ++i){
				face.pos_index[i]=face.tex_index[i]=face.nor_index[i]=0;	// no value
				// format might be:
				// f 1 2 3
				// f 1/1/1 2/2/2 3/3/3 4/4/4
				// f 1//1 2//2 3//3 4//4
				// only assuming 3 faces
                str_stream >> face.pos_index[i];
				// there might be a '/'
				if ('/'==str_stream.peek())
				{
					str_stream.ignore();	// skip it
					if ('/'!=str_stream.peek())	// not "//"
					{
						str_stream >> face.tex_index[i];
					}
					if ('/'==str_stream.peek())
					{
						str_stream.ignore();	// skip it
						str_stream >> face.nor_index[i];
					}
				}
				// OBJ format is 1+, C++ is 0+ so we must -1 from all indexes
				// however: sometimes the indexes can be -ve relative the current index
				if (face.pos_index[i] < 0)	// -ve
					face.pos_index[i]=positions.size()+face.pos_index[i];	// relative to end
				else
					face.pos_index[i]--;	// reduce by 1 for 0+ indexes
				if (face.tex_index[i] < 0)	// -ve
					face.tex_index[i]=texcoords.size()+face.tex_index[i];	// relative to end
				else
					face.tex_index[i]--;	// reduce by 1 for 0+ indexes
				if (face.nor_index[i] < 0)	// -ve
					face.nor_index[i]=normals.size()+face.nor_index[i];	// relative to end
				else
					face.nor_index[i]--;	// reduce by 1 for 0+ indexes
            }
            faces.push_back(face);
		}
    }
	// Explicit closing of the file
	fclose(file);
	return faces.size()>0;	// must load at least once face
}

bool ModelLoad::SimpleLoadMtl(const wchar_t* filename,std::vector<Material>& mats)
{
	// loader based upon above code & http://www.fileformat.info/format/material/
	const ModelLoad::Vector3f BLACK={0,0,0};
	const Material NEW_MTRL={L"",BLACK,BLACK,BLACK,1,32,L""};

	Material newmtrl=NEW_MTRL;
	
	const wchar_t* TOKEN_NAME=L"newmtl";
	const wchar_t* TOKEN_AMBIENT=L"Ka";
	const wchar_t* TOKEN_DIFFUSE=L"Kd";
	const wchar_t* TOKEN_SPECULAR=L"Ks";
	const wchar_t* TOKEN_ALPHA1=L"d",*TOKEN_ALPHA2=L"Tr";	// two possibles
	const wchar_t* TOKEN_SPECULAR_POWER=L"Ns";
	const wchar_t* TOKEN_DIFFUSE_TEX=L"map_Kd";

	FILE* file=nullptr;
	_wfopen_s(&file,filename,L"r");
	if (file==nullptr)	return false;
	wchar_t line_stream[1024];
	while(!feof(file))
	{
		// performance improvement:
		// by changing from std::getline() to fgetws() time taken is ~25% of original!!!
		fgetws(line_stream,1024,file);
		std::wstringstream str_stream(line_stream);
		std::wstring type_str;
        str_stream >> type_str;
        if(type_str == TOKEN_NAME){
			if (newmtrl.matMame.size()>0)	// there is a previous material
			{
				mats.push_back(newmtrl);	// add
				newmtrl=NEW_MTRL;
			}
			str_stream>>newmtrl.matMame;
		}else if (type_str == TOKEN_AMBIENT){
			str_stream>>newmtrl.ambient.x>>newmtrl.ambient.y>>newmtrl.ambient.z;
		}else if (type_str == TOKEN_DIFFUSE){
			str_stream>>newmtrl.diffuse.x>>newmtrl.diffuse.y>>newmtrl.diffuse.z;
		}else if (type_str == TOKEN_SPECULAR){
			str_stream>>newmtrl.specular.x>>newmtrl.specular.y>>newmtrl.specular.z;
		}else if (type_str == TOKEN_DIFFUSE_TEX){
			str_stream>>newmtrl.texFilename;
		}else if (type_str == TOKEN_SPECULAR_POWER){
			str_stream>>newmtrl.shinyness;
		}else if (type_str == TOKEN_ALPHA1 || type_str == TOKEN_ALPHA2){
			str_stream>>newmtrl.alpha;
		}
	}
	if (newmtrl.matMame.size()>0)	// there is a previous material
		mats.push_back(newmtrl);	// add
	// Explicit closing of the file
	fclose(file);
	return mats.size()>0;	// must load at least once material
}

}	// namespace