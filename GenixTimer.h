#pragma once
#include <string>

class GenixTimer
{
public:
	// Thread-Safe Initialization of a (Meyers) Singleton
	static GenixTimer& GetInstance()
	{
		static GenixTimer instance;
		return instance;
	}

	GenixTimer(GenixTimer &&) = delete;
	GenixTimer(GenixTimer const&) = delete;
	void operator=(GenixTimer const&) = delete;
	void operator=(GenixTimer &&) = delete;

	float	TotalTime()const; // in seconds
	float	DeltaTime()const; // in seconds

	void	Init();
	void	Reset(); // Call before message loop.
	void	Start(); // Call when unpaused.
	void	Stop();  // Call when paused.
	void	Tick();  // Call every frame.
private:
	GenixTimer() { Init(); };
	~GenixTimer() = default;

	static  GenixTimer* instance;

	bool	bStopped		{ false };

	double	dDeltaTime		{ -1 };
	double	dSecondsPerCount{ 0 };

	__int64 BaseTime		{ 0 };
	__int64 PausedTime		{ 0 };
	__int64 StopTime		{ 0 };
	__int64 PrevTime		{ 0 };
	__int64 CurrTime		{ 0 };

};

