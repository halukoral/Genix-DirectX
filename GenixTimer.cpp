#include "Genix.h"
#include "GenixTimer.h"

GenixTimer* GenixTimer::instance = nullptr;

void GenixTimer::Init()
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)& countsPerSec);
	dSecondsPerCount = 1.0 / (double)countsPerSec;
}

// Returns the total time elapsed since Reset() was called, 
// NOT counting any time when the clock is stopped.
float GenixTimer::TotalTime()const
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// mStopTime - mBaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from mStopTime:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (bStopped)
	{
		return (float)(((StopTime - PausedTime) - BaseTime) * dSecondsPerCount);
	}

	// The distance mCurrTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from mCurrTime:  
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime

	else
	{
		return (float)(((CurrTime - PausedTime) - BaseTime) * dSecondsPerCount);
	}
}

float GenixTimer::DeltaTime()const
{
	return (float)dDeltaTime;
}

void GenixTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)& currTime);

	BaseTime = currTime;
	PrevTime = currTime;
	StopTime = 0;
	bStopped = false;
}

void GenixTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)& startTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (bStopped)
	{
		PausedTime += (startTime - StopTime);

		PrevTime = startTime;
		StopTime = 0;
		bStopped = false;
	}
}

void GenixTimer::Stop()
{
	if (!bStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)& currTime);

		StopTime = currTime;
		bStopped = true;
	}
}

void GenixTimer::Tick()
{
	if (bStopped)
	{
		dDeltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)& currTime);
	CurrTime = currTime;

	// Time difference between this frame and the previous.
	dDeltaTime = (CurrTime - PrevTime) * dSecondsPerCount;

	// Prepare for next frame.
	PrevTime = CurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (dDeltaTime < 0.0)
	{
		dDeltaTime = 0.0;
	}
}