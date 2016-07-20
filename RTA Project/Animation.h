#pragma once
#include <string>
#include "Importer.h"

class Animation
{
public:
	Animation();
	~Animation();
private:
	float duration;

	Importer::Vertex vertices[];
};

