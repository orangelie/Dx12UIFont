#include "GameTimer.h"

namespace orangelie
{
	CLASSIFICATION_C2(GameTimer,
		{
			INT64 ticksPerSecond;
			QueryPerformanceFrequency((LARGE_INTEGER*)&ticksPerSecond);
			mSecondsPerTick = (1.0f / ticksPerSecond);

			GameTimer::mDeltaTime = -1.0f;
			GameTimer::mIsStopped = false;
			GameTimer::mBaseTime = GameTimer::mCurrTime = GameTimer::mPausedTime = GameTimer::mPrevTime = GameTimer::mStopTime = 0;
		});

	void GameTimer::Tick()
	{
		if (mIsStopped)
		{
			mDeltaTime = 0.0f;
			return;
		}

		INT64 currTime{ 0 };
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mCurrTime = currTime;
		mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerTick;
		mPrevTime = currTime;

		if (mDeltaTime <= 0.0f)
		{
			mDeltaTime = 0.0f;
			return;
		}
	}

	void GameTimer::Reset()
	{
		INT64 currTime{ 0 };
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mBaseTime = mCurrTime = mPrevTime = currTime;
		mStopTime = 0;
		mIsStopped = false;
	}

	void GameTimer::Start()
	{
		if (mIsStopped)
		{
			INT64 currTime{ 0 };
			QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

			mPausedTime += currTime - mStopTime;
			mPrevTime = currTime;
			mStopTime = 0;
			mIsStopped = false;
		}
	}

	void GameTimer::Stop()
	{
		if (!mIsStopped)
		{
			INT64 currTime{ 0 };
			QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

			mStopTime = currTime;
			mIsStopped = true;
		}
	}

	float GameTimer::DeltaTime() const
	{
		return mDeltaTime;
	}

	float GameTimer::TotalTime() const
	{
		if (mIsStopped)
		{
			return (((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerTick);
		}
		
		return (((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerTick);
	}
}