#pragma once
#include "Importer.h"

class Interpolator
{

public:
	Interpolator();
	~Interpolator();

	inline void AddTime(float timeToAdd) { currentTime += timeToAdd; }
	inline void SetTime(float _currentTime) { currentTime = _currentTime; }

	// Creates the "betweenKeyFrame" data based on the animation and currentTime elements
	void Process();

private:
	float currentTime;

	// The animation this interpolator is interpolating
	Importer::Animation *animPtr;

	// The result of the interpolation, if not using channels
	Importer::KeyFrame betweenKeyFrame;

	// Or if using channels, we would have one result per joint
	//KeyFrame betweenKeyFrameChannel[NUMBER_OF_JOINTS_IN_RIG]
};

