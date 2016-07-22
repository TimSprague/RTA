#pragma once
#include "Importer.h"

class Interpolator
{
public:
	Interpolator();
	~Interpolator();

	inline void AddTime(float timeToAdd) { currentTime += timeToAdd; }
	inline void SetTime(float _currentTime) { currentTime = _currentTime; }
	FbxAMatrix LERP(FbxAMatrix* _matrix1, FbxAMatrix* _matrix2, float _time);

	// Creates the "betweenKeyFrame" data based on the animation and currentTime elements
	void Process(Importer::Animation* anim);

private:
	float currentTime;

	// The animation this interpolator is interpolating
	Importer::Animation *animPtr;

	// The result of the interpolation, if not using channels
	Importer::KeyFrame betweenKeyFrame;

};

