#include "D3DApp.h"

// The user-provided entry point for a graphical Windows-based application.
// WinMain is the conventional name used for the application entry point.
int CALLBACK WinMain(
	HINSTANCE hInstance,	 // A handle to the current instance of the application.
	HINSTANCE hPrevInstance, // This parameter is always NULL.
	LPSTR     lpCmdLine,	 // The command line for the application, excluding the program name. 
							 // To retrieve the entire command line, use the GetCommandLine function.
	int       nShowCmd
)
{
	try
	{
		return D3DApp{}.Run();
	}
	catch (const GenixException& e)
	{
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}
