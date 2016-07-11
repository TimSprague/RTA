#pragma once
#include "stdafx.h"
#include "Triangle.h"
//#include "fbxsdk.h"
#include <fbxsdk.h>
#pragma comment (lib,"libfbxsdk-md.lib")
#pragma comment (lib,"wininet.lib")

class FBXexporter
{
	friend class Triangle;
	// how many triangles
	unsigned int mTriangleCount = 0;

	// vector of triangles
	std::vector<Triangle> mTriangles;

	//

	void ProcessMesh(FbxNode* inNode);

	void ProcessControlPoints(FbxNode* inNode);

	void ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outNormal);

	void ProcessSkeletonHierarchy(FbxNode* inRootNode);

	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex);

	void ProcessJointsAndAnimations(FbxNode* inNode);
};



