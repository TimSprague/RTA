#include "Interpolator.h"

Interpolator::Interpolator()
{

}

void Interpolator::Process(Importer::Animation* anim)
{
	animPtr = anim;

	// loop through the keyframes (60 for the idle animation)
	FbxTime totalAnimationTime;
	for (unsigned int i = 0; i < 60; i++)
	{
		totalAnimationTime += (FbxLongLong)animPtr[i].duration;
	}

	for (int currIndex = 0; currIndex < 60; currIndex++)
	{
		int prevIndex = currIndex - 1;
		if (prevIndex == -1)
		{
			prevIndex = 0;
		}
		for (FbxLongLong j = 0; j < totalAnimationTime.GetFrameCount(FbxTime::eFrames24); j++)
		{
			if (j == 0)
			{
				SetTime(0);
			}
			else
			{
				AddTime((float)j);
			}

			if (currentTime > animPtr[prevIndex].duration &&currentTime < animPtr[currIndex].duration)
			{
				betweenKeyFrame.FrameNum = currIndex;
				betweenKeyFrame.globalTransform = LERP(&animPtr[prevIndex].keyframes[prevIndex].globalTransform, &animPtr[currIndex].keyframes[currIndex].globalTransform, animPtr[currIndex].duration);
				betweenKeyFrame.next = &(*betweenKeyFrame.next);
			}

		}
	}


}

FbxAMatrix Interpolator::LERP(FbxAMatrix* _matrix1, FbxAMatrix* _matrix2, float _time)
{
	FbxAMatrix temp;
	float ratio = _time / currentTime;
	for (int matrixRow = 0; matrixRow < 4; matrixRow++)
	{
		for (int  matrixColumn = 0; matrixColumn < 4; matrixColumn++)
		{
			temp.mData[matrixRow].mData[matrixColumn] = _matrix1->mData[matrixRow].mData[matrixColumn] + (_matrix2->mData[matrixRow].mData[matrixColumn] - _matrix1->mData[matrixRow].mData[matrixColumn]) * ratio;
		}
	}

	return temp;
}

Interpolator::~Interpolator()
{
}
