#include "Importer.h"

Importer::Importer()
{

}

void Importer::ImportPolygons(FbxMesh* inNode)
{
	if (inNode == nullptr)
	{
		return;
	}

	// number of control points
	int numControlPoints = inNode->GetControlPointsCount();
	// fills the vector with float4s to be changed 
	controlPoints.resize(numControlPoints);


	// store control points
	for (int i = 0; i < numControlPoints; i++)
	{
		controlPoints[i].x = (float)inNode->GetControlPointAt(i).mData[0];
		controlPoints[i].y = (float)inNode->GetControlPointAt(i).mData[1];
		controlPoints[i].z = (float)inNode->GetControlPointAt(i).mData[2];
	}

	// number of polygons in this mesh
	polygonCount = inNode->GetPolygonCount();
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
			int controlPointIndex = inNode->GetPolygonVertex(i, j);
			// storing the postion of the vertex based on the vertex in the control point
			tempVerts.position = controlPoints[controlPointIndex];

			// read the first layer, the first texture
			FbxLayerElementUV* tempUV = inNode->GetLayer(0)->GetUVs();
			// check which the uv was mapped to and pass to the tempVertex.UV
			switch (tempUV->GetMappingMode())
			{
			case FbxLayerElement::eByControlPoint:
			{
				tempVerts.UV.x = (float)tempUV->GetDirectArray().GetAt(controlPointIndex).mData[0];
				tempVerts.UV.y = 1.0 - (float)tempUV->GetDirectArray().GetAt(controlPointIndex).mData[1];
			}
			break;
			case FbxLayerElement::eByPolygonVertex:
			{
				// get the information at the uv on the current uv on the texture of the polygon
				tempVerts.UV.x = (float)tempUV->GetDirectArray().GetAt(inNode->GetTextureUVIndex(i, j)).mData[0];
				tempVerts.UV.y = 1.0 - (float)tempUV->GetDirectArray().GetAt(inNode->GetTextureUVIndex(i, j)).mData[1];
			}
			break;
			default:
				break;
			}
			// get the normal from the polygon
			FbxVector4 tempNormal;
			inNode->GetPolygonVertexNormal(i, j, tempNormal);
			// map into tempVert.normal
			tempVerts.normal.x = (float)tempNormal.mData[0];
			tempVerts.normal.y = (float)tempNormal.mData[1];
			tempVerts.normal.z = (float)tempNormal.mData[2];

			// replace the vertx with the tempVert
			totalVertexes[vertexCounter] = tempVerts;
			uniqueVertices[vertexCounter] = tempVerts;
			uniqueIndicies.push_back(vertexCounter);

			/*if (uniqueVertices.size() == 0)
			{
				uniqueVertices.push_back(tempVerts);
				uniqueIndicies.push_back(0);
				vertexCounter++;
				continue;
			}

			for (unsigned int k = 0; k < uniqueVertices.size(); ++k)
			{
				if (   uniqueVertices[k].position.x == tempVerts.position.x
					&& uniqueVertices[k].position.y == tempVerts.position.y
					&& uniqueVertices[k].position.z == tempVerts.position.z
					&& uniqueVertices[k].normal.x == tempVerts.normal.x
					&& uniqueVertices[k].normal.y == tempVerts.normal.y
					&& uniqueVertices[k].normal.z == tempVerts.normal.z
					&& uniqueVertices[k].UV.x == tempVerts.UV.x
					&& uniqueVertices[k].UV.y == tempVerts.UV.y)
				{

					uniqueIndicies.push_back(k);
					continue;
				}
				else
				{
					uniqueVertices.push_back(tempVerts);
					uniqueIndicies.push_back(uniqueVertices.size()-1);
				}
			}*/
			vertexCounter++;
			
		}
	}
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
			if (stat(file.c_str(), &st) == 0)
			{
				FileOpen(file);
			}
			else
			{
				ImportPolygons(scene->GetRootNode()->GetChild(i)->GetMesh());
			}

			break;
		}
	}

	FileSave(_filename + ".bin");
}

//void Importer::FindUniqueIndices()
//{
//	int vertexSize = (int)totalVertexes.size();
//	Vertex temp;
//	for (int i = 0; i < vertexSize; i++)
//	{
//		for (int j = 0; j < vertexSize; j++)
//		{
//			if (i != j)
//			{
//				if (totalVertexes[i].position.x == totalVertexes[j].position.x 
//					&& totalVertexes[i].position.y == totalVertexes[j].position.y
//					&& totalVertexes[i].position.z == totalVertexes[j].position.z
//					&& totalVertexes[i].normal.x == totalVertexes[j].normal.x
//					&& totalVertexes[i].normal.y == totalVertexes[j].normal.y
//					&& totalVertexes[i].normal.z == totalVertexes[j].normal.z
//					&& totalVertexes[i].UV.x == totalVertexes[j].UV.x
//					&& totalVertexes[i].UV.y == totalVertexes[j].UV.y)
//				{
//					continue;
//				}
//				else
//				{
//					temp.position.y = totalVertexes[j].position.y;
//					temp.position.z = totalVertexes[j].position.z;
//					temp.normal.x = totalVertexes[j].normal.x;
//					temp.normal.y = totalVertexes[j].normal.y;
//					temp.normal.z = totalVertexes[j].normal.z;
//					temp.UV.x = totalVertexes[j].UV.x;
//					temp.UV.y = totalVertexes[j].UV.y;
//					uniqueVertices.push_back(temp);
//					uniqueIndicies.push_back(j);
//				}
//			}
//		}
//	}
//
//}

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
		bout.write((char*)&controlPoints[0], sizeof(DirectX::XMFLOAT3)*size);

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
		bin.read((char*)&controlPoints[0], size * sizeof(DirectX::XMFLOAT3));

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

Importer::~Importer()
{
}
