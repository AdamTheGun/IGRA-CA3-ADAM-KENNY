/*==============================================
 * Overloads for SimpleMath & XMFLOAT in std::wostream
 *
 * Written by Mark Gossage
 *==============================================*/
#pragma once
#include <ostream>
#include <stdio.h>
#include <SimpleMath.h>

inline std::wostream& operator<<(std::wostream& os,const DirectX::XMFLOAT2& v)
{
	wchar_t buff[256];
	swprintf_s(buff,256,L"(%5.2f,%5.2f)",v.x,v.y);
	os<<buff;
	return os;
}

inline std::wostream& operator<<(std::wostream& os,const DirectX::XMFLOAT3& v)
{
	wchar_t buff[256];
	swprintf_s(buff,256,L"(%5.2f,%5.2f,%5.2f)",v.x,v.y,v.z);
	os<<buff;
	return os;
}

inline std::wostream& operator<<(std::wostream& os,const DirectX::XMFLOAT4& v)
{
	wchar_t buff[256];
	swprintf_s(buff,256,L"(%5.2f,%5.2f,%5.2f,%5.2f)",v.x,v.y,v.z,v.w);
	os<<buff;
	return os;
}
