#pragma once
#include "Genix.h"
#include "GenixException.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include "DxgiInfoManager.h"

// To set up Direct3D we need to complete the following four steps:
// 1. Define the device types and feature levels we want to check for.
// 2. Create the Direct3D device, rendering context, and swap chain.
// 3. Create the render target.
// 4. Set the viewport.

// In Direct3D 11 we can have a hardware device, a WARP device, a software
// driver device, or a reference device.
// -A hardware device is a Direct3D device that runs on the graphics hardware and
// is the fastest of all devices
// -A reference device is used for users without hardware support by performing the
// rendering on the CPU. In other words, the reference device completely emulates
// hardware rendering on the CPU within software. This process is very slow,
// inefficient, and should only be used during development if there is no other
// alternative.
// -A software driver device allows developers to write their own software
// rendering driver and use it with Direct3D. This is called a pluggable software
// driver.
// -The WARP device is an efficient CPU-rendering device that emulates the full
// feature set of Direct3D.

// The next step is the creation of the swap chain description.
// A swap chain is a collection of buffers that are used for displaying frames 
// to the user. Each time an application presents a new frame for display, 
// the first buffer in the swap chain takes the place of the displayed buffer. 
// This process is called swapping or flipping.

// The next step is to create the rendering context, device, and swap chain now that
// we have the swap chain description. The Direct3D device is the device itself and
// communicates with the hardware. The Direct3D context is a rendering context
// that tells the device how to draw. It also includes rendering states and other
// drawing information. The swap chain as we’ve already discussed is the rendering
// destinations that the device and context will draw to.

// A render target view is a Direct3D resource written to by the output merger
// stage. In order for the output merger to render to the swap chain’s back buffer
// (secondary buffer), we create a render target view of it.

// The last piece of the Direct3D 11 puzzle is the creation and setting of the
// viewport. The viewport defines the area of the screen we are rendering to. In
// single player or non-split-screen multiplayer games this is often the entire
// screen, and so we set the viewport’s width and height to the Direct3D swap
// chain’s width and height.
class Graphics
{
public:
	class Exception : public GenixException { using GenixException::GenixException; };
	
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what()					const noexcept override;
		const char* GetType()				const noexcept override;
		HRESULT		GetErrorCode()			const noexcept;
		std::string GetErrorString()		const noexcept;
		std::string GetErrorDescription()	const noexcept;
		std::string GetErrorInfo()			const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class InfoException : public Exception
	{
	public:
		InfoException( int line,const char* file,std::vector<std::string> infoMsgs ) noexcept;
		const char* what()			const noexcept override;
		const char* GetType()		const noexcept override;
		std::string GetErrorInfo()	const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};

public:
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;
	
	void	EndFrame();
	void	ClearBuffer(float red, float green, float blue) noexcept;
	void 	DrawTestTriangle();

	bool	Get4xMsaaState() const { return b4xMsaaState; }
	void	Set4xMsaaState(bool value);

private:

#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	// Set true to use 4X MSAA
	bool b4xMsaaState{ true };    // 4X MSAA enabled
	UINT i4xMsaaQuality{ 0 };      // quality level of 4X MSAA

	static constexpr int ScreenWidth = 1280;
	static constexpr int ScreenHeight = 720;
	/**
	 * You can create a swap chain by calling 
	 * IDXGIFactory2::CreateSwapChainForHwnd, 
	 * IDXGIFactory2::CreateSwapChainForCoreWindow, or 
	 * IDXGIFactory2::CreateSwapChainForComposition. 
	 * 
	 * You can also create a swap chain when you call 
	 * D3D11CreateDeviceAndSwapChain; 
	 * however, you can then only access the 
	 * sub-set of swap-chain functionality that 
	 * the IDXGISwapChain interface provides.
	 */

	/**
	 *					WHAT IS ComPtr?
	 * Creates a smart pointer type that represents 
	 * the interface specified by the template parameter. 
	 * ComPtr automatically maintains a reference count 
	 * for the underlying interface pointer and releases 
	 * the interface when the reference count goes to zero.
	 * https://docs.microsoft.com/tr-tr/cpp/cppcx/wrl/comptr-class?view=vs-2019
	 */

	// one or more surfaces for storing rendered 
	// data before presenting it to an output.
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;

	// virtual adapter it is used to create resources. 
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;	

	// a device context which generates rendering commands.
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;

	// identifies the render-target subresources 
	// that can be accessed during rendering.
	// A rendertarget is a resource that can be written by the 
	// output-merger stage at the end of a render pass. Each 
	// render-target should also have a corresponding depth-stencil view.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;

	Microsoft::WRL::ComPtr<IDXGIFactory> pDXGIFactory;
	Microsoft::WRL::ComPtr<IDXGIDevice>  pDXGIDevice;
	Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
};
