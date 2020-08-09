#include "Window.h"
#include <sstream>
#include "resource.h"
#include "WindowsThrowMacros.h"

/**
	*							WHAT IS HINSTANCE?
	*  A handle to an instance. This is the base address of the module in memory.
	*  HMODULE and HINSTANCE are the same today, but represented different things 
	*  in 16-bit Windows.
	*  typedef HANDLE HINSTANCE;
	*  typedef HINSTANCE HMODULE;
	*  https://devblogs.microsoft.com/oldnewthing/20040614-00/?p=38903
	*/

// GetModuleHandle: This function returns a module handle for the specified 
// module if the file has been mapped into the address space of the calling process.
Window::Window() : hInst( GetModuleHandle( nullptr ) )
{
	// register window class
	pWc = new WNDCLASSEX();

	pWc->cbSize = sizeof(WNDCLASSEX);

	// Allocates a unique device context 
	// for each window in the class. 
	pWc->style = CS_OWNDC;

	// A pointer to the window procedure. 
	// You must use the CallWindowProc function 
	// to call the window procedure. 
	pWc->lpfnWndProc = HandleMsgSetup;

	pWc->cbClsExtra = 0;
	pWc->cbWndExtra = 0;

	// A handle to the instance that contains  
	// the window procedure for the class.
	pWc->hInstance = hInst;

	pWc->hIcon = static_cast<HICON>(
		LoadImage(
			hInst, MAKEINTRESOURCE(IDI_ICON1),
			IMAGE_ICON, 32, 32, 0
		));
	pWc->hCursor = nullptr;

	// A handle to the class background brush.
	pWc->hbrBackground = nullptr;

	pWc->lpszMenuName = nullptr;

	// A pointer to a null-terminated string or is an atom. 
	// If this parameter is an atom, it must be a class atom 
	// created by a previous call to the RegisterClass or 
	// RegisterClassEx function. The atom must be in the low-order 
	// word of lpszClassName; the high-order word must be zero.
	pWc->lpszClassName = pClassName;

	pWc->hIconSm = static_cast<HICON>(
		LoadImage(
			hInst, MAKEINTRESOURCE(IDI_ICON1),
			IMAGE_ICON, 16, 16, 0
		));

	// Registers a window class for subsequent use in 
	// calls to the CreateWindow or CreateWindowEx function.
	// The RegisterClassEx() must be called before we
	// attempt to create the window, and it takes as a parameter the address of the
	// window class structure to register. If a value of 0 is returned by this function,
	// then the registration failed, and you’ll likely need to check the values specified
	// for the window class to ensure they are all valid
	RegisterClassEx(pWc);
}

Window::Window(HINSTANCE hInstance) : Window()
{
	// Create Window Instance: Creates an overlapped, pop-up, 
	// or child window with an extended window style; otherwise, 
	// this function is identical to the CreateWindow function
	hWnd = CreateWindowEx(
		0, pClassName,"Window",
		// The style of the window being created. This parameter can be a combination
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, 
		CW_USEDEFAULT,CW_USEDEFAULT, width, height,
		nullptr, nullptr, hInstance, 
		this // Pointer to a value to be passed to the window through the 
					// CREATESTRUCT structure (lpCreateParams member) pointed to 
					// by the lParam param of the WM_CREATE message. This message 
					// is sent to the created window by this function before it returns.
	);

	// check for error
	if (hWnd == nullptr)
	{
		throw GHWND_LAST_EXCEPT();
	}
	// show window
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	// create graphics object
	pGfx = std::make_unique<Graphics>(hWnd);
}

Window::Window(int w, int h, const char* name) : Window()
{
	// The next step is to create the actual window. First we call AdjustWindowRect to
	// calculate the size required of the window based on our desired dimensions and style.
	width = w;
	height = h;
	// calculate window size based on desired client region size
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
		
	// The AdjustWindowRect function first takes a rectangle (lpRect) that defines the
	// bottom left, bottom right, upper left, and upper right areas of the window. The
	// left and top properties represent the starting location of the window, while the
	// right and bottom represent the width and height. The AdjustWindowRect
	// function also takes as parameters the window style flag of the window being
	// created and a Boolean flag indicating whether or not the window has a menu,
	// which affects the non-client area.
	if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
	{
		throw GHWND_LAST_EXCEPT();
	}
	// create window & get hWnd
	// The window is created next by calling the Win32 function CreateWindow.
	hWnd = CreateWindow(
		pClassName, name,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, hInst, this
	);
	// check for error
	if (hWnd == nullptr)
	{
		throw GHWND_LAST_EXCEPT();
	}
	// show window
	// The return value of the CreateWindow(A) function is a non-null handle. If
	// CreateWindow(A) succeeds, we can show the window by calling the Win32
	// function ShowWindow, which takes as parameters the window handle returned by
	// CreateWindow(A) and the command show flag
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	// create graphics object
	pGfx = std::make_unique<Graphics>(hWnd);

	// With the window created, the application can begin doing its job. Win32 GUI
	// applications are event-based applications. This essentially means that when an
	// event happens, the application is notified of it, and some action then occurs.
	// This continues until the event for quitting the application is encountered.

}

Window::~Window()
{
	UnregisterClass(pClassName, pWc->hInstance);
	DestroyWindow(hWnd);
	delete pWc;
}

LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// use create parameter passed in from CreateWindow() 
	// to store window class pointer at WinAPI side
	if (msg == WM_NCCREATE)
	{
		// extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
			
		// set WinAPI-managed user data to store ptr to window class
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			
		// set message procedure to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
			
		// forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	// if we get a message before the WM_NCCREATE message, handle with default handler
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WINAPI Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// retrieve ptr to window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	// forward message to window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	switch (msg)
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bAppPaused = true;
			//Timer.Stop();
		}
		else
		{
			bAppPaused = false;
			//Timer.Start();
		}
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		if (1) //md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				bAppPaused = true;
				bMinimized = true;
				bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				bAppPaused = false;
				bMinimized = false;
				bMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (bMinimized)
				{
					bAppPaused = false;
					bMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (bMaximized)
				{
					bAppPaused = false;
					bMaximized = false;
					OnResize();
				}
				else if (bResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		bAppPaused = true;
		bResizing = true;
		//Timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		bAppPaused = false;
		bResizing = false;
		//Timer.Start();
		OnResize();
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 800;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 600;
		return 0;

	// we don't want the DefProc to handle this message because
	// we want our destructor to destroy the window, so return 0 instead of break
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	// clear keystate when window loses focus to prevent input getting "stuck"
	case WM_KILLFOCUS:
		kbd.ClearState();
		break;
		
	/*********** KEYBOARD MESSAGES ***********/
	case WM_KEYDOWN:
	// syskey commands need to be handled to track ALT key (VK_MENU) and F10
	case WM_SYSKEYDOWN:
		if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled()) // filter autorepeat
		{
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		kbd.OnChar(static_cast<unsigned char>(wParam));
		break;
	/*********** END KEYBOARD MESSAGES ***********/

	/************* MOUSE MESSAGES ****************/
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height)
		{
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.IsInWindow())
			{
				SetCapture(hWnd);
				mouse.OnMouseEnter();
			}
		}
		// not in client -> log move / maintain capture if button down
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON))
			{
				mouse.OnMouseMove(pt.x, pt.y);
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftPressed(pt.x, pt.y);
		// bring window to foreground on lclick client region
		SetForegroundWindow(hWnd);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightPressed(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(pt.x, pt.y);
		// release mouse if outside of window
		if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height)
		{
			ReleaseCapture();
			mouse.OnMouseLeave();
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightReleased(pt.x, pt.y);
		// release mouse if outside of window
		if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height)
		{
			ReleaseCapture();
			mouse.OnMouseLeave();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		mouse.OnWheelDelta(pt.x, pt.y, delta);
		break;
	}
	/************** END MOUSE MESSAGES **************/
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

Graphics& Window::Gfx()
{
	if(!pGfx)
		throw GHWND_NOGFX_EXCEPT();
	return *pGfx;
}

void Window::OnResize()
{

}

void Window::SetTitle(const std::string& title)
{
	if (SetWindowText(hWnd, title.c_str()) == 0)
	{
		throw GHWND_LAST_EXCEPT();
	}
}

std::optional<int> Window::ProcessMessages()
{
	MSG msg;

	// With window messages we need to do two things. First we need to get new
	// messages and process them, and second we need to dispatch (respond) to these
	// messages. The PeekMessage Win32 function retrieves a message for the associated
	// window (the window we’ve created using CreateWindow). The first
	// parameter is the structure that will hold the message (its address), the window
	// handle (optional), min and max message filter flags (optional), and the remove
	// flag. Specifying PM_REMOVE as the remove flag like we’ve done removes it from the
	// queue. Since we are processing this message, it will not need to stay on the queue
	// once we’ve handled it.
	// while queue has messages, remove and dispatch them (but do not block on empty queue)
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// check for quit because peekmessage does not signal this via return val
		if (msg.message == WM_QUIT)
		{
			// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
			return (int)msg.wParam;
		}

		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
		// If there is a message obtained by PeekMessage, we can respond to that message
		// by calling TranslateMessage and DispatchMessage. The Win32 function
		// TranslateMessage translates the messages from virtual-key messages to character
		// messages, and the DispatchMessage function dispatches the message to
		// the Windows procedure callback function. The Windows procedure function 
		// will actually perform actions based on the message it receives.
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// return empty optional when not quitting app
	return {};
}


// Window Exception Stuff
std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
	);
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}


Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{}

const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "Chili Window Exception";
}

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return Exception::TranslateErrorCode(hr);
}


const char* Window::NoGfxException::GetType() const noexcept
{
	return "Chili Window Exception [No Graphics]";
}