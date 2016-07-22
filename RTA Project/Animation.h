#pragma once
#include <string>
#include "Importer.h"

class Animation
{
public:
	Animation();
	~Animation();
private:

	string m_name;
	float duration;

	Importer::KeyFrame keyframes[];
};

