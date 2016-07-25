#pragma once
#include "stdafx.h"
//#include "fbxsdk.h"
#include <fbxsdk.h>
//#pragma comment (lib,"libfbxsdk-md.lib")
//#pragma comment (lib,"wininet.lib")


class Importer
{
	friend class Interpolator;
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 UV;
		DirectX::XMFLOAT3 normal;

		// array for bones per vertex
		int bones[4] = { -1,-1,-1,-1 };
		// the weight of how much each bone effects the movements
		float weight[4] = { 0,0,0,0 };
	};

	struct KeyFrame
	{
		FbxLongLong FrameNum;
		FbxAMatrix globalTransform;
		KeyFrame* next;

		KeyFrame() : next(nullptr) {}
	};

	struct Joint
	{
		string name;
		int parentIndex;
		FbxAMatrix globalBindposeInverse;
		//KeyFrame* animation;
		FbxNode* node;
		vector<KeyFrame> animation;

		Joint() : node(nullptr)//, animation(nullptr)
		{
			globalBindposeInverse.SetIdentity();
			parentIndex = -1;
		}

		~Joint()
		{
			//while (animation)
			//{
			//	KeyFrame* temp = animation->next;
			//	delete animation;
			//	animation = temp;
			//}
		}
	};

	struct Skeleton
	{
		vector<Joint> joints;
	};

	struct BlendingIndexWeightPair
	{
		int blendingIndex[4] = { -1,-1,-1,-1 };
		float blendingWeight[4] = { 0,0,0,0 };
	};

	struct CtrlPoint
	{
		DirectX::XMFLOAT3 postion;
		vector<BlendingIndexWeightPair> blendingInfo;
	};

	struct Animation
	{
		string name;
		float duration;
		KeyFrame keyframes[60];
	};

	vector<CtrlPoint> controlPoints;
	vector<Vertex> totalVertexes;
	vector<Vertex> uniqueVertices;
	vector<UINT> uniqueIndicies;
	Skeleton skeleton;
	int polygonCount;
	vector<FbxMesh*> meshes;
	Animation mAnimation;
	FbxString mAnimationName;
	FbxTime mAnimationLength;

	Importer();
	void ImportPolygons(FbxNode* inNode);
	void ImportFile(std::string _filename);
	unsigned int FindJointUsingName(string inString);
	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int index, int inParentIndex);
	FbxAMatrix GetGeometryTransformation(FbxNode* inNode);
	void ProcessJointAndAnimation(FbxNode* inNode, FbxMesh* inMesh);
	void ProcessSkeletonHierarchy(FbxNode* inNode);
	void NormalizeVectors(float* inVert);
	void Animate();
	void FileSave(string _filename);
	void FileOpen(string _filename);
	~Importer();

private:

	/*vector<DirectX::XMFLOAT3> controlPoints;
	vector<Vertex> totalVertexes;
	int polygonCount;*/
};

