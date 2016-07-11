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
		controlPoints[i].x = inNode->GetControlPointAt(i).mData[0];
		controlPoints[i].y = inNode->GetControlPointAt(i).mData[1];
		controlPoints[i].z = inNode->GetControlPointAt(i).mData[2];
	}


	// number of polygons in this mesh
	polygonCount = inNode->GetPolygonCount();
	totalVertexes.resize(polygonCount * 3);

	int vertexCounter = 0;
	// loop through the number of triangles (polygons)
	for (unsigned int i = 0; i < polygonCount; ++i)
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
				tempVerts.UV.x = tempUV->GetDirectArray().GetAt(controlPointIndex).mData[0];
				tempVerts.UV.y = tempUV->GetDirectArray().GetAt(controlPointIndex).mData[1];
			}
			break;
			case FbxLayerElement::eByPolygonVertex:
			{
				// get the information at the uv on the current uv on the texture of the polygon
				tempVerts.UV.x = tempUV->GetDirectArray().GetAt(inNode->GetTextureUVIndex(i, j)).mData[0];
				tempVerts.UV.y = tempUV->GetDirectArray().GetAt(inNode->GetTextureUVIndex(i, j)).mData[1];
			}
			break;
			default:
				break;
			}
			// get the normal from the polygon
			FbxVector4 tempNormal;
			inNode->GetPolygonVertexNormal(i, j, tempNormal);
			// map into tempVert.normal
			tempVerts.normal.x = tempNormal.mData[0];
			tempVerts.normal.y = tempNormal.mData[1];
			tempVerts.normal.z = tempNormal.mData[2];

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
	importer->Initialize(_filename.c_str,-1,manager->GetIOSettings());
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


Importer::~Importer()
{
}
