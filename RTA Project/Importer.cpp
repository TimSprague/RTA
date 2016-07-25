#include "Importer.h"
#include "Interpolator.h"
Importer::Importer()
{

}

void Importer::ImportPolygons(FbxNode* inNode)
{
	if (inNode == nullptr)
	{
		return;
	}

	FbxMesh* currMesh = inNode->GetMesh();
	meshes.push_back(currMesh);
	ProcessSkeletonHierarchy(inNode);
	if (currMesh == nullptr)
	{
		ProcessJointAndAnimation(inNode,meshes[0]);
		return;
	}
	// number of control points
	int numControlPoints = currMesh->GetControlPointsCount();
	// fills the vector with float4s to be changed 
	controlPoints.resize(numControlPoints);


	// store control points
	for (int i = 0; i < numControlPoints; i++)
	{
		controlPoints[i].postion.x = (float)currMesh->GetControlPointAt(i).mData[0];
		controlPoints[i].postion.y = (float)currMesh->GetControlPointAt(i).mData[1];
		controlPoints[i].postion.z = (float)currMesh->GetControlPointAt(i).mData[2];
	}

	// number of polygons in this mesh
	polygonCount = currMesh->GetPolygonCount();
	totalVertexes.resize(polygonCount * 3);
	uniqueVertices.resize(totalVertexes.size());

	int vertexCounter = 0;
	// loop through the number of triangles (polygons)
	for (int i = 0; i < polygonCount; ++i)
	{
		// loop through the triangle vertices
		for (unsigned int j = 0; j < 3; j++)
		{
			Vertex tempVerts;

			// the current location on which triangle triangle
			int controlPointIndex = currMesh->GetPolygonVertex(i, j);
			// storing the postion of the vertex based on the vertex in the control point
			tempVerts.position = controlPoints[controlPointIndex].postion;


			// read the first layer, the first texture
			FbxLayerElementUV* tempUV = currMesh->GetLayer(0)->GetUVs();
			// check which the uv was mapped to and pass to the tempVertex.UV
			switch (tempUV->GetMappingMode())
			{
			case FbxLayerElement::eByControlPoint:
			{
				tempVerts.UV.x = (float)tempUV->GetDirectArray().GetAt(controlPointIndex).mData[0];
				tempVerts.UV.y = (float)1.0 - (float)tempUV->GetDirectArray().GetAt(controlPointIndex).mData[1];
			}
			break;
			case FbxLayerElement::eByPolygonVertex:
			{
				// get the information at the uv on the current uv on the texture of the polygon
				tempVerts.UV.x = (float)tempUV->GetDirectArray().GetAt(currMesh->GetTextureUVIndex(i, j)).mData[0];
				tempVerts.UV.y = (float)1.0 - (float)tempUV->GetDirectArray().GetAt(currMesh->GetTextureUVIndex(i, j)).mData[1];
			}
			break;
			default:
				break;
			}
			// get the normal from the polygon
			FbxVector4 tempNormal;
			currMesh->GetPolygonVertexNormal(i, j, tempNormal);
			// map into tempVert.normal
			tempVerts.normal.x = (float)tempNormal.mData[0];
			tempVerts.normal.y = (float)tempNormal.mData[1];
			tempVerts.normal.z = (float)tempNormal.mData[2];

			// replace the vertx with the tempVert
			totalVertexes[vertexCounter] = tempVerts;
			uniqueVertices[vertexCounter] = tempVerts;
			uniqueIndicies.push_back(vertexCounter);

			vertexCounter++;
			
		}
	}
	/*Interpolator temp;
	temp.Process(&mAnimation);*/
}

void Importer::ImportFile(string _filename)
{
	if (_filename[0] == '/0')
	{
		return;
	}
	string file = _filename + ".bin";
	struct stat st;

	// create a manager
	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* inputOutput = FbxIOSettings::Create(manager, "objectPointerToName");

	manager->SetIOSettings(inputOutput);
	// create importer and scene to access the information from the file
	FbxImporter* importer = FbxImporter::Create(manager, "fbxImporter");
	FbxScene* scene = FbxScene::Create(manager, "fbxScene");

	// import the files scene
	importer->Initialize(_filename.c_str(), -1, manager->GetIOSettings());
	importer->Import(scene, false); // document is the scene pointer
	importer->Destroy(true);

	// get the number of children to loop for
	int numChildren = scene->GetRootNode()->GetChildCount(false);

	for (int i = 0; i < numChildren; i++)
	{
		if (scene->GetRootNode()->GetChild(i)->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			/*if (stat(file.c_str(), &st) == 0)
			{
				FileOpen(file);
			}
			else
			{
				ImportPolygons(scene->GetRootNode()->GetChild(i));
				continue;
			}*/
			ImportPolygons(scene->GetRootNode()->GetChild(i));

		}
		ImportPolygons(scene->GetRootNode()->GetChild(i));
	}
	//FileSave(_filename + ".bin");

}

FbxAMatrix Importer::GetGeometryTransformation(FbxNode* inNode)
{
	FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

unsigned int Importer::FindJointUsingName(string inString)
{
	if (inString == "Root_J")
	{
		return 0;
	}
	// looping through the wrong thing to find the name, maybe find out which CLUSTER we are
	
	unsigned int numBones = skeleton.joints.size();

	for (unsigned int i = 0; i < numBones; i++)
	{
		
		string name = skeleton.joints[i].name.c_str();
		if (strcmp(name.c_str(), inString.c_str()) == 0)
			return i;
	}

	
	return UINT32_MAX;
}

void Importer::ProcessJointAndAnimation(FbxNode* inNode, FbxMesh* inMesh)
{
	FbxMesh* currmesh = inMesh;
	unsigned int numDeformers = currmesh->GetDeformerCount();
	FbxAMatrix geomTransform = GetGeometryTransformation(inNode);

	for (unsigned int deformerIndex = 0; deformerIndex < numDeformers; deformerIndex++)
	{
		FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currmesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

		unsigned int	numClusters = currSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numClusters; clusterIndex++)
		{
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			string currJointName = currCluster->GetLink()->GetName();
			unsigned int currJointIndex = FindJointUsingName(currJointName);

			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindposeInverseMatrix;

			currCluster->GetTransformMatrix(transformMatrix);
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geomTransform;

			skeleton.joints[currJointIndex].globalBindposeInverse = globalBindposeInverseMatrix;
			skeleton.joints[currJointIndex].node = currCluster->GetLink();

			unsigned int numIndicies = currCluster->GetControlPointIndicesCount();
			for (unsigned int i = 0; i < numIndicies; i++)
			{

				BlendingIndexWeightPair currBlending;

					for (int j = 0; j < 4; j++)
					{
						// if it is empty fill it in with the next index and weight
						if (currBlending.blendingIndex[j] == -1)
						{
							currBlending.blendingIndex[j] = currJointIndex;
							currBlending.blendingWeight[j] = (float)currCluster->GetControlPointIndices()[i];
						}
					}

				controlPoints[currCluster->GetControlPointIndices()[i]].blendingInfo.push_back(currBlending);
			}

			FbxScene* currScene = inNode->GetScene();
			FbxAnimStack* currAnimStack = currScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			mAnimationName = animStackName.Buffer();
			FbxTakeInfo* takInfo = currScene->GetTakeInfo(animStackName);
			FbxTime start = takInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takInfo->mLocalTimeSpan.GetStop();
			mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;

			//KeyFrame** currAnim = &skeleton.joints[currJointIndex].animation;
			mAnimation.name = mAnimationName;
			mAnimation.duration = (float)end.GetFrameCount(FbxTime::eFrames24);
			skeleton.joints[currJointIndex].animation.resize(mAnimation.duration);

			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i < end.GetFrameCount(FbxTime::eFrames24); i++)
			{
				// get current time between the frams
				FbxTime currTime;
				currTime.SetFrame(i, FbxTime::eFrames24);
				// create a new keyframe for the list
				//*currAnim = new KeyFrame();
				// set the identifier for later reference
				//(*currAnim)->FrameNum = i;
				skeleton.joints[currJointIndex].animation[i].FrameNum = i;
				// set the offset for the transform matrix
				FbxAMatrix currTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geomTransform;
				//(*currAnim)->globalTransform = currTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
				skeleton.joints[currJointIndex].animation[i].globalTransform = currTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
				// store animation in the keyframe
				//mAnimation.keyframes[i] = **currAnim;
				//mAnimation.keyframes[i].next = &mAnimation.keyframes[i+1];
				// position to the next node in the list, wait to be written
				//currAnim = &((*currAnim)->next);
			}

		}
	}

	// normalize the weights 
	for (unsigned int i = 0; i < totalVertexes.size(); i++)
	{
		NormalizeVectors(totalVertexes[i].weight);
	}

}

void Importer::NormalizeVectors(float* inVert)
{
	float sum = 0.0000000000001f;

	sum = inVert[0] + inVert[1] + inVert[2] + inVert[2];

	inVert[0] /= sum; inVert[1] /= sum; inVert[2] /= sum; inVert[3] /= sum;

	// make sure that the total doesn't span over 1
	inVert[3] = 1.0f -( inVert[0] - inVert[1] - inVert[2]);

}

void Importer::ProcessSkeletonHierarchy(FbxNode* inNode)
{
	int count = inNode->GetChildCount();
	FbxMesh* currmesh = inNode->GetMesh();
	if (count == 0)
	{
		return;
	}

	for (int i = 0; i < count; i++)
	{
		FbxNode* currNode = inNode->GetChild(i);
		ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
	}

}

void Importer::ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int index, int inParentIndex)
{
	if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Joint tempJoint;
		tempJoint.parentIndex = inParentIndex;
		tempJoint.name = inNode->GetName();
		skeleton.joints.push_back(tempJoint);
	}

	int test = inNode->GetChildCount();
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, (int)skeleton.joints.size(), index);
	}
}

void Importer::FileSave(string _filename)
{
	ofstream bout;

	bout.open(_filename.c_str(), ios_base::binary);

	int size = (int)controlPoints.size();
	int size1 = (int)totalVertexes.size();
	int size2 = (int)uniqueVertices.size();
	int size3 = (int)uniqueIndicies.size();

	if (bout.is_open())
	{
		bout.write((char*)&polygonCount, sizeof(int));

		bout.write((char*)&size, sizeof(int));
		bout.write((char*)&controlPoints[0], sizeof(CtrlPoint)*size);

		bout.write((char*)&size1, sizeof(int));
		bout.write((char*)&totalVertexes[0], sizeof(Vertex)*size1);

		bout.write((char*)&size2, sizeof(int));
		bout.write((char*)&uniqueVertices[0], sizeof(Vertex)*size2);

		bout.write((char*)&size3, sizeof(int));
		bout.write((char*)&uniqueIndicies[0], sizeof(UINT) * size3);

		bout.close();
	}
}

void Importer::FileOpen(string _filename)
{
	ifstream bin;

	int size;
	int size1;
	int size2;
	int size3;

	bin.open(_filename.c_str(), ios_base::binary);

	if (bin.is_open())
	{
		bin.read((char*)&polygonCount, sizeof(int));

		bin.read((char*)&size, sizeof(int));
		controlPoints.resize(size);
		bin.read((char*)&controlPoints[0], size * sizeof(CtrlPoint));

		bin.read((char*)&size1, sizeof(int));
		totalVertexes.resize(size1);
		bin.read((char*)&totalVertexes[0], size1 * sizeof(Vertex));

		bin.read((char*)&size2, sizeof(int));
		uniqueVertices.resize(size2);
		bin.read((char*)&uniqueVertices[0], size2 * sizeof(Vertex));

		bin.read((char*)&size3, sizeof(int));
		uniqueIndicies.resize(size3);
		bin.read((char*)&uniqueIndicies[0], size3 * sizeof(UINT));

		bin.close();
	}
}

void Importer::Animate()
{
	unsigned int numFrames = (unsigned int)mAnimation.duration;

	for (unsigned int frame = 0; frame < numFrames; frame++)
	{
		for (unsigned int jointIndex = 0; jointIndex < skeleton.joints.size(); jointIndex++)
		{
			skeleton.joints[jointIndex].animation[frame].globalTransform;
		}
	}
}

Importer::~Importer()
{
}
