#pragma once
#include "stdafx.h"
#define BACKBUFFER_WIDTH	1280
#define BACKBUFFER_HEIGHT	800
#define PIXELS ((BACKBUFFER_WIDTH)*(BACKBUFFER_HEIGHT))
#define PI 3.14159f

struct OBJVERTEX
{
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT3 uv;
	DirectX::XMFLOAT3 normals;
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

struct DIRECTIONAL_LIGHT
{
	DirectX::XMFLOAT4 directionaldir;
	DirectX::XMFLOAT4 directionalcolor;
	DirectX::XMMATRIX directionalMatrix;
};

struct CUBE
{
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR rgba;
};

struct LINE
{
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR rgba;
};

struct SKYBOX
{
	DirectX::XMVECTOR pos;
};








