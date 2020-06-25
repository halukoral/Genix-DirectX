#pragma once
#include "Genix.h"
#include "GenixException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <optional>
#include <memory>

class Window
{
public:
	class Exception : public GenixException
	{
	public:
		Exception(int line, const char* file) noexcept
			: GenixException(line, file), hr(0) {}

		Exception(int line, const char* file, HRESULT hr) noexcept
			: GenixException(line, file), hr(hr) {}

				const char* what()			const noexcept override;
		virtual const char* GetType()		const noexcept	{ return "Genix Window Exception"; }
			
			HRESULT GetErrorCode()			const noexcept	{ return hr; }
		std::string GetErrorDescription()	const noexcept	{ return TranslateErrorCode(hr); }

		static std::string TranslateErrorCode(HRESULT hr) noexcept;
			
	private:
		// A 32-bit value that is used to describe an error or warning.
		HRESULT hr;
	};

	class NoGfxException : public Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override { return "Genix Window Exception [No Graphics]"; }
	};
public:
	Window();
	~Window();
	Window( int width,int height,const char* name );
	Window(HINSTANCE hInstance);

	Window (const Window&) = delete;
	Window& operator=(const Window&) = delete;

	Graphics&	Gfx();
	void		OnResize();
	void		SetTitle (const std::string& title);
	float		AspectRatio() const { return static_cast<float>(width) / height; }
		
	static 		std::optional<int> ProcessMessages ();

	Mouse		mouse;
	Keyboard	kbd;

private:

	/**
		*							WHAT IS CALLBACK?
		*  CALLBACK, WINAPI, and APIENTRY are all used to define functions 
		*  with the __stdcall calling convention. Most functions in the 
		*  Windows API are declared using WINAPI. You may wish to use 
		*  CALLBACK for the callback functions that you implement to help 
		*  identify the function as a callback function.
		*
		*  Do you curious about __stdcall? Ok then:
		*  https://stackoverflow.com/questions/297654/what-is-stdcall
		*  
		*  The __stdcall calling convention is used to call Win32 API functions. 
		*  The callee cleans the stack, so the compiler makes vararg functions __cdecl. 
		*  Functions that use this calling convention require a function prototype. 
		*  The __stdcall modifier is Microsoft-specific.
		*  
		*  LRESULT: Signed result of message processing.
		*/

	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		   LRESULT			HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	int				width		{ 1280 };
	int				height		{ 720 };

	bool			bAppPaused = false;
	bool			bMinimized = false;
	bool			bMaximized = false;
	bool			bResizing = false;
	bool			bFullscreenState = false;

	// msg: Contains message information from a thread's message queue.
	MSG				msg {0};

	// A handle to window ( a window is a rectangular area of the screen 
	// where the application displays output and receives input from the user)
	// https://docs.microsoft.com/en-us/windows/win32/winmsg/windows
	HWND			hWnd;
		
	BOOL			bResult		{ 0 };
		
	// Contains window class information. It is used with the 
	// RegisterClassEx() and GetClassInfoEx()  functions.
	WNDCLASSEX*		pWc			{ nullptr };
		
	HINSTANCE		hInst		{ nullptr };

	// LPCSTR: A pointer to a constant null-terminated string of 8-bit Windows (ANSI) characters.
	const LPCSTR	pClassName	{ "Direct3D Engine Window" };
		
	std::unique_ptr<Graphics> pGfx;
};

// error exception helper macro
#define GHWND_EXCEPT( hr )	 Window::Exception( __LINE__,__FILE__,hr )
#define GHWND_LAST_EXCEPT()  Window::Exception( __LINE__,__FILE__,GetLastError() )
#define GHWND_NOGFX_EXCEPT() Window::NoGfxException( __LINE__,__FILE__ )
