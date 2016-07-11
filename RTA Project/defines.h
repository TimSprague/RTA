#pragma once
#include <DirectXMath.h>

#define BACKBUFFER_WIDTH	1280
#define BACKBUFFER_HEIGHT	800
#define PIXELS ((BACKBUFFER_WIDTH)*(BACKBUFFER_HEIGHT))
#define PI 3.14159f

UINT Raster[PIXELS];

float aspectRatio = (float)(BACKBUFFER_WIDTH) / (BACKBUFFER_HEIGHT);
float zNear = 0.1f;
float zFar = 100.0f;
float zBuffer[PIXELS];

struct OBJVERTEX
{
	float4 pos;
	float3 uv;
	float3 normals;
};

struct OBJECT
{
	DirectX::XMMATRIX worldMatrix;
};

struct SCENE
{
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projMatrix;
};

// Functions
float Degrees_to_Radian(float Deg)
{
	return Deg * PI / 180.0f;
}


