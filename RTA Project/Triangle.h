#pragma once
#include "stdafx.h"
class Triangle
{
	friend class FBXexporter;
	std::vector<unsigned int> Indicies;
	vector<DirectX::XMFLOAT3> Points;
public:
	Triangle();
	~Triangle();
};

