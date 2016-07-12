#pragma once
#include "stdafx.h"
//#include "fbxsdk.h"
#include <fbxsdk.h>
#pragma comment (lib,"libfbxsdk-md.lib")
#pragma comment (lib,"wininet.lib")

class Importer
{

public:

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 UV;

		// array for bones per vertex
		int bones[4] = { -1,-1,-1,-1 };
		// the weight of how much each bone effects the movements
		float weight[4] = { 0,0,0,0 };
	};

	vector<DirectX::XMFLOAT3> controlPoints;
	vector<Vertex> totalVertexes;
	int polygonCount;

	Importer();
	void ImportPolygons(FbxMesh* inNode);
	void ImportFile(std::string _filename);
	~Importer();

private:

	/*vector<DirectX::XMFLOAT3> controlPoints;
	vector<Vertex> totalVertexes;
	int polygonCount;*/
};

