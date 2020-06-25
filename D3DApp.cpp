#include "D3DApp.h"
#include <sstream>

D3DApp::D3DApp() : wnd(1280, 720, "The DirectX 11")
{
	Timer = &GenixTimer::GetInstance();
}

int D3DApp::Run()
{
	Timer->Reset();
	while (true)
	{
		// process all messages pending, but to not block for new messages
		if (const auto ecode = Window::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		Timer->Tick();
		DoFrame();
	}
}

void D3DApp::DoFrame()
{
	wnd.Gfx().ClearBuffer(0.f, 0.f, 1.0f);
	wnd.Gfx().DrawTestTriangle();
	wnd.Gfx().EndFrame();
}