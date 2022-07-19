#pragma once

#include "Utils.h"

namespace orangelie
{
	class GameTimer
	{
	public:
		CLASSIFICATION_H(GameTimer);

		void Tick();

		void Reset();
		void Start();
		void Stop();

		float DeltaTime() const;
		float TotalTime() const;

	private:
		INT64 mBaseTime, mPausedTime, mStopTime, mCurrTime, mPrevTime;

		float mDeltaTime, mSecondsPerTick;

		bool mIsStopped;

	};
}