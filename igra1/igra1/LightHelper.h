/*==============================================
 * IGRA Light helpers
 *
 * Written by IGRA team
 *==============================================*/
#pragma once

#include "DxCommon.h"	// Common Dx stuff


namespace igra
{

	struct Material
	{
		DirectX::SimpleMath::Color Ambient;
		DirectX::SimpleMath::Color Diffuse;
		DirectX::SimpleMath::Color Specular; // w = SpecPower
		DirectX::SimpleMath::Color Reflect;
	};

	struct DirectionalLight
	{
		DirectX::SimpleMath::Color Ambient;
		DirectX::SimpleMath::Color Diffuse;
		DirectX::SimpleMath::Color Specular;
		DirectX::SimpleMath::Vector3 Direction;
		float pad;
	};

	inline Material MakeMaterial(const float col[],const float spec[]=DirectX::SimpleMath::Color(1,1,1),float power=16)
	{
		Material mat;
		mat.Diffuse=mat.Ambient=DirectX::SimpleMath::Color(col);
		mat.Specular=DirectX::SimpleMath::Color(spec);
		mat.Specular.w=power;
		mat.Reflect=DirectX::SimpleMath::Color(0,0,0,0);
		return mat;
	}



}	// namespace
