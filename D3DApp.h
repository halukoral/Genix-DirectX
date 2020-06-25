#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Window.h"
#include "GenixTimer.h"

class D3DApp
{
public:
	D3DApp();
	int	Run();

	// Used to keep track of the “delta-time”
	GenixTimer*	Timer;

private:
	void DoFrame();	
	// main window 
	Window wnd;
};