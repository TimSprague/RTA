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
				tempVerts.UV.y = (float)tempUV->GetDirectArray().GetAt(controlPointIndex).mData[1];
			}
			break;
			case FbxLayerElement::eByPolygonVertex:
			{
				// get the information at the uv on the current uv on the texture of the polygon
				tempVerts.UV.x = (float)tempUV->GetDirectArray().GetAt(inNode->GetTextureUVIndex(i, j)).mData[0];
				tempVerts.UV.y = (float)tempUV->GetDirectArray().GetAt(inNode->GetTextureUVIndex(i, j)).mData[1];
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
	// create a manager
	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* inputOutput = FbxIOSettings::Create(manager,"objectPointerToName");
	
	manager->SetIOSettings(inputOutput);
	// create importer and scene to access the information from the file
	FbxImporter* importer = FbxImporter::Create(manager, "fbxImporter");
	FbxScene* scene = FbxScene::Create(manager, "fbxScene");

	// import the files scene
	importer->Initialize(_filename.c_str(),-1,manager->GetIOSettings());
	importer->Import(scene,false); // document is the scene pointer
	importer->Destroy(true);
	
	// get the number of children to loop for
	int numChildren = scene->GetRootNode()->GetChildCount(false);

	for (int i = 0; i < numChildren; i++)
	{
		if (scene->GetRootNode()->GetChild(i)->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			ImportPolygons(scene->GetRootNode()->GetChild(i)->GetMesh());
			break;
		}
	}
	
}

void Importer::FileSave(string _filename)
{
	ofstream bout;

	bout.open(_filename.c_str(), ios_base::binary);

	vector<Importer> fbxVectors;
	Importer temp;

	temp.controlPoints = controlPoints;
	temp.polygonCount = polygonCount;
	temp.totalVertexes = totalVertexes;
	fbxVectors.push_back(temp);

	int size = (int)fbxVectors.size();

	if (bout.is_open())
	{
		bout.write((char *)&size, sizeof(int));
		bout.write((char*)&fbxVectors[0], sizeof(Importer)*fbxVectors.size());

		bout.close();
	}
	
}

void Importer::FileOpen(string _filename)
{
	ifstream bin;

	vector<Importer> fbxVectors;
	int size;
	bin.open(_filename.c_str(), ios_base::binary);

	if (bin.is_open())
	{
		bin.read((char*)&size, sizeof(int));
		fbxVectors.resize(size);
		bin.read((char*)&fbxVectors[0], size * sizeof(Importer));

		bin.close();
	}

	controlPoints = fbxVectors[0].controlPoints;
	polygonCount = fbxVectors[0].polygonCount;
	totalVertexes = fbxVectors[0].totalVertexes;
}



Importer::~Importer()
{
}
