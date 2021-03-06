#pragma once
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
#include <iostream>
#include <Windows.h>
#include <cmath>
#include <math.h>
#include <stdio.h>
#include <tchar.h>
#include <cstring>
#include <cstdlib>
#include <conio.h>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <atlbase.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <fbxsdk.h>
#include "defines.h"
#include "DDSTextureLoader.h"
#include "XTime.h"

//Shaders
#include "RTA_VS.csh"
#include "RTA_PS.csh"
#include "Lines_VS.csh"
#include "Lines_PS.csh"
#include "Instance_VS.csh"
#include "Instance_lines_VS.csh"
#include "Skybox_VS.csh"
#include "Skybox_PS.csh"


#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

using namespace std;
//#pragma comment (lib, "d3d11.lib")


