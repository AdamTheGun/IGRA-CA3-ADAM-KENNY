/*==============================================
 * Common DX related stuff for IGRA Engine
 *
 * Written by IGRA Team
 *==============================================*/
#pragma once

// debugging routines
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// windows stuff without all the extra bits
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// DX
#include <d3d11.h>

// stl
#include <vector>
#include <string>

// the COM Smart pointer
#include <atlbase.h>

// DirectXTK items
#include "SimpleMath.h"
#include "CommonStates.h"

