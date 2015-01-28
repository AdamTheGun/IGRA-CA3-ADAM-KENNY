/*==============================================
 * Heightmap loader & generator
 *
 * Written by <YOUR NAME>
 *==============================================*/
#include "Terrain.h"
#include "IgraApp.h"

#include "IgraUtils.h"

#include "ToString.h"
#include "Vertex.h"


namespace igra
{

Terrain::Terrain(ID3D11Device* pDev,const wchar_t* filename,const DirectX::XMFLOAT3& scale)
{
	// store info
	mpDevice=pDev;
	mTerrainScale=scale;
	// load the heightmap
	HeightMapLoad(filename);
	// create buffers
	mpVertexBuffer=CreateTerrainVertexBuffer(pDev);
	mpIndexBuffer=CreateTerrainIndexBuffer(pDev);
}

Terrain::~Terrain()
{
}

void Terrain::Draw(ID3D11DeviceContext* pContext)
{
	//FAIL("TODO: Terrain::Draw()");
	// this is easy:

	ID3D11Buffer*  verts[]={mpVertexBuffer};
	UINT stride = sizeof(TerrainVertex); //size of 1x Vertex in bytes
	UINT offset = 0;
	
	pContext->IASetVertexBuffers(0,1,verts, &stride, &offset);

	pContext->IASetIndexBuffer(mpIndexBuffer,DXGI_FORMAT_R32_UINT,0);
	pContext->DrawIndexed(mNumIndex,0,0);
}


void Terrain::HeightMapLoad(const wchar_t* filename)
{
	// quick hack to get it working:
	// mTerrainHeight=mTerrainWidth=257;
	// mHeightMap.resize(mTerrainHeight*mTerrainWidth,250.0f);
	// return;
	FILE *filePtr=nullptr;	// Pointer to the current position in the file
	BITMAPFILEHEADER bitmapFileHeader;	
                   // Structure which stores information about file
	BITMAPINFOHEADER bitmapInfoHeader;		
                  // Structure which stores information about image
	// Open the file
	_wfopen_s(&filePtr,filename,L"rb");
	if (filePtr == NULL)
		FAIL(filename);
	// Read bitmaps header & info
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1,filePtr);
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	// Get the width and height (width and length) of the image
	mTerrainWidth = bitmapInfoHeader.biWidth;
	mTerrainHeight = bitmapInfoHeader.biHeight;

	unsigned int lineWidth = ((bitmapInfoHeader.biWidth *
                               bitmapInfoHeader.biBitCount / 8) + 3) & ~3;
	int imageSize = lineWidth * mTerrainHeight;
	// Initialize the array which stores the image data
	unsigned char* bitmapImage = new unsigned char[imageSize];
	// Set the file pointer to the beginning of the image data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
	// read image data into bitmapImage
	fread(bitmapImage, 1, imageSize, filePtr);
	// Close file
	fclose(filePtr);

	// Initialize the heightMap array (stores the vertices of our terrain)
	mHeightMap.resize(mTerrainWidth * mTerrainHeight);
	// Read the image data into our heightMap array
	for(int j=0; j< mTerrainHeight; j++)
	{
		for(int i=0; i< mTerrainWidth; i++)
		{
			// index in the mHeightMap
			int index = ( mTerrainHeight * j) + i;
			// index in the bitmap info
			int k=( lineWidth * j) + i*bitmapInfoHeader.biBitCount / 8;
			mHeightMap[index]=bitmapImage[k];
		}
	}
	delete [] bitmapImage;
}

float Terrain::GetHeight(float x, float z)
{
	// pre-calc sized
	float width=mTerrainWidth*mTerrainScale.x;
	float height=mTerrainHeight*mTerrainScale.z;

	// Get the row and column we are in.
	float c=(x+ 0.5f* width)/mTerrainScale.x;
	float d=(z+ 0.5f* height)/mTerrainScale.z;

	// convert x,z into integer row/col
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// range check for sanity:
	if (row<0 || row>=mTerrainWidth-1 || col<0 || col>=mTerrainHeight-1)
       	return 0;

	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D
	float A = mHeightMap[row*mTerrainWidth+ col];
	float B = mHeightMap[row*mTerrainWidth + col + 1];
	float C = mHeightMap[(row+1)*mTerrainWidth + col];
	float D = mHeightMap[(row+1)*mTerrainWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if( s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return (A + s*uy + t*vy) * mTerrainScale.y;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return (D + (1.0f-s)*uy + (1.0f-t)*vy) * mTerrainScale.y;
	}
}

ID3D11Buffer* Terrain::CreateTerrainVertexBuffer(ID3D11Device* pDev)
{
	//FAIL("TODO: Terrain::CreateTerrainVertexBuffer()");
	// refer to the notes
	// rows & cols for ease
	int rows=mTerrainWidth, cols=mTerrainHeight;
	// compute size of the map, so we can centre it
	float width=rows*mTerrainScale.x;
	float depth=cols*mTerrainScale.z;
	// number of vertexes
	int numVerts=rows*cols;
	std::vector<TerrainVertex> vertexes(numVerts);
	int index=0;
	for(int j=0; j< cols; j++)
	{
		for(int i=0; i< rows; i++)
		{
			index = ( rows * j) + i;
			// We divide the height by this number to "water down" the 
			// terrains height, otherwise the terrain will
			// appear to be "spikey" and not so smooth.
			// usually scale.y will be 0.1 or similar
			vertexes[index].Position.x=i*mTerrainScale.x - width/2;	
			// in centre of map
			vertexes[index].Position.y=mHeightMap[index]*
                                                         mTerrainScale.y;
			vertexes[index].Position.z=j*mTerrainScale.z - depth/2;				// in centre of map
			vertexes[index].TexC.x=(float)(i)/(rows-1.0f);
			vertexes[index].TexC.y=(float)(cols-j)/(cols-1.0f);
		}
	}
	// make buffers:
	return CreateVertexBuffer(pDev,&vertexes[0],
                                     sizeof(TerrainVertex)*vertexes.size());

	return nullptr;
}

ID3D11Buffer* Terrain::CreateTerrainIndexBuffer(ID3D11Device* pDev)
{
	//FAIL("TODO: Terrain::CreateTerrainIndexBuffer()");
	// refer to the notes
	// rows & cols for ease
	int rows=mTerrainWidth, cols=mTerrainHeight;
	// for a N*N area there will be only (N-1)*(N-1) quads
	// and you need 2 triangles (6 indexes) to make one quad
	mNumIndex=6*(rows-1)*(cols-1);
	std::vector<UINT> indexes;
	indexes.reserve(mNumIndex);	// make sure there is enough space
	for(int j=0; j< cols-1; j++)
	{
		for(int i=0; i< rows-1; i++)
		{
			// add the indexes
			// Bottom left, Bottom right, Top left of quad
			indexes.push_back(i+(j*rows));
			indexes.push_back(i+((j+1)*rows));
			indexes.push_back(i+1+(j*rows));
			// Top left, Bottom right, Top right of quad
			indexes.push_back(i+((j+1)*rows));
			indexes.push_back(i+1+((j+1)*rows));
			indexes.push_back(i+1+(j*rows));
		}
	}

	// make buffers:
	return CreateIndexBuffer(pDev,&indexes[0],indexes.size());

	return nullptr;
}


DirectX::XMFLOAT3 Terrain::GetPosOnTerrain(DirectX::XMFLOAT3 pos,float offset)
{
	pos.y=GetHeight(pos.x,pos.z)+offset;
	return pos;
}


}	// namespace