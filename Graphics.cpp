#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")
namespace wrl = Microsoft::WRL;

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Graphics::InfoException( __LINE__,__FILE__,v);}}
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

Graphics::Graphics(HWND hWnd)
{
	/**
	 *					WHAT IS SWAP CHAIN?
	 * A swap chain is a series of virtual framebuffers utilized 
	 * by the graphics card and graphics API for frame rate 
	 * stabilization and several other functions. The swap 
	 * chain usually exists in graphics memory, but it can exist 
	 * in system memory as well. The non-utilization of a swap 
	 * chain may result in stuttering rendering, but its existence 
	 * and utilization are required by many graphics APIs. A swap 
	 * chain with two buffers is a double buffer.
	 * 
	 * A swap chain is a collection of buffers that are used for 
	 * displaying frames to the user. Each time an application presents 
	 * a new frame for display, the first buffer in the swap chain takes 
	 * the place of the displayed buffer. This process is called swapping or flipping.
	 * https://docs.microsoft.com/en-us/windows/win32/direct3d9/what-is-a-swap-chain-
	 */

	// This structure is used by the GetDesc and CreateSwapChain methods.
	// In full-screen mode, there is a dedicated front buffer; 
	// in windowed mode, the desktop is the front buffer.
	DXGI_SWAP_CHAIN_DESC sd {};

	// BufferDesc: describes the backbuffer display mode.
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb173064(v=vs.85)
	sd.BufferDesc.Width = Graphics::ScreenWidth;
	sd.BufferDesc.Height = Graphics::ScreenHeight;
	
	//A DXGI_RATIONAL structure describing the refresh rate in hertz
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	
	//A DXGI_FORMAT structure describing the display format.
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

	// enumerated type describing the scanline drawing mode.
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	
	// enumerated type describing the scaling mode.
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// SampleDesc: describes multi-sampling parameters.
	// https://docs.microsoft.com/en-us/windows/win32/api/dxgicommon/ns-dxgicommon-dxgi_sample_desc
	// The number of multisamples per pixel.
	sd.SampleDesc.Count = b4xMsaaState ? 4 : 1;

	// The image quality level. The higher the quality, 
	// the lower the performance. The valid range is 
	// between zero and one less than the level returned by 
	// Direct3D 11 (ID3D11Device::CheckMultisampleQualityLevels)
	sd.SampleDesc.Quality = b4xMsaaState ? (i4xMsaaQuality - 1) : 0;

	// BufferUsage: A member of the DXGI_USAGE enumerated 
	// type that describes the surface usage and CPU access 
	// options for the back buffer. The back buffer can be 
	// used for shader input or render-target output.
	// enumurated type that describes surface usage and 
	// CPU access options for back buffer which is used 
	// for sharder input or render target output.
	// https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-usage
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	
	sd.BufferCount = 1;

	// An HWND handle to the output window. This member must not be NULL.
	sd.OutputWindow = hWnd;
	
	sd.Windowed = TRUE;

	// enumerated type that describes options for handling the contents 
	// of the presentation buffer after presenting a surface.
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// for checking results of d3d functions
	HRESULT hr;

	//////////////////////////////////////////////////////////////////////////
	// Creates a device that represents the display 
	// adapter and a swap chain used for rendering.
	// D3D11CreateDeviceAndSwapChain(...)
	//
	// IDXGIAdapter: A pointer to the video adapter to use when creating a device.
	//	
	// HMODULE: A handle to a DLL that implements a software rasterizer. 
	// If DriverType is D3D_DRIVER_TYPE_SOFTWARE, Software must not be NULL.
	//	
	// D3D_FEATURE_LEVEL: A pointer to an array of D3D_FEATURE_LEVELs, 
	// which determine the order of feature levels to attempt to create.
	//
	// IDXGISwapChain: Returns the address of a pointer to the IDXGISwapChain 
	// object that represents the swap chain used for rendering.
	//
	// D3D_FEATURE_LEVEL: Returns a pointer to a D3D_FEATURE_LEVEL, which represents 
	// the first element in an array of feature levels supported by the device.
	//
	// create device and front/back buffers, and swap chain and rendering context
	//////////////////////////////////////////////////////////////////////////
	GFX_THROW_INFO( D3D11CreateDeviceAndSwapChain(
		nullptr,					/* IDXGIAdapter* */
		D3D_DRIVER_TYPE_HARDWARE,	/* D3D_DRIVER_TYPE */
		nullptr,					/* HMODULE */
		0,							/* UINT */
		nullptr,					/* const D3D_FEATURE_LEVEL */
		0,							/* UINT */
		D3D11_SDK_VERSION,			/* UINT */
		&sd,						/* const DXGI_SWAP_CHAIN_DESC* */
		&pSwap,						/* IDXGISwapChain *  */
		&pDevice,					/* ID3D11Device * */
		nullptr,					/* D3D_FEATURE_LEVEL */
		&pContext					/* ID3D11DeviceContext* */
	));

	// A resource interface provides common actions on all resources.
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	
	// Accesses one of the swap-chain's back buffers.
	// https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-getbuffer
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));

	// Creates a render-target view for accessing resource data.
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(
		pBackBuffer.Get(), // represents a render target.
		nullptr, // nullptr -> create a view that accesses all of the subresources in mipmap level 0.
		&pTarget
	));
	pBackBuffer->Release();
}

void Graphics::EndFrame()
{
	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	// flip back/front buffers
	if (FAILED(hr = pSwap->Present(1u, 0u)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		throw GFX_EXCEPT(hr);
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red,green,blue,1.0f };
	// Set all the elements in a render target to one value.
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::Set4xMsaaState(bool value)
{
	if (b4xMsaaState != value)
	{
		b4xMsaaState = value;

		// Recreate the swapchain and buffers with new multisample settings.
		//CreateSwapChain();
		//OnResize();
	}
}

void Graphics::DrawTestTriangle()
{
	namespace wrl = Microsoft::WRL;
	HRESULT hr;

	struct Vertex
	{
		float x;
		float y;
	};

	// create vertex buffer (1 2d triangle at center of screen)
	const Vertex vertices[] =
	{
		{ 0.0f,0.5f },
		{ 0.5f,-0.5f },
		{ -0.5f,-0.5f },
	};

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof( vertices );
	bd.StructureByteStride = sizeof( Vertex );
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;
	GFX_THROW_INFO( pDevice->CreateBuffer( &bd,&sd,&pVertexBuffer ) );

	// Bind vertex buffer to pipeline
	const UINT stride = sizeof( Vertex );
	const UINT offset = 0u;
	pContext->IASetVertexBuffers( 0u,1u,pVertexBuffer.GetAddressOf(),&stride,&offset );

	wrl::ComPtr<ID3DBlob> pBlob;
	
	// create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO( D3DReadFileToBlob( L"VertexShader.cso",&pBlob ) );
	GFX_THROW_INFO( pDevice->CreateVertexShader( pBlob->GetBufferPointer(),pBlob->GetBufferSize(),nullptr,&pVertexShader ) );

	// bind vertex shader
	pContext->VSSetShader( pVertexShader.Get(),nullptr,0u );

	// create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	GFX_THROW_INFO( D3DReadFileToBlob( L"PixelShader.cso",&pBlob ) );
	GFX_THROW_INFO( pDevice->CreatePixelShader( pBlob->GetBufferPointer(),pBlob->GetBufferSize(),nullptr,&pPixelShader ) );

	// bind pixel shader
	pContext->PSSetShader( pPixelShader.Get(),nullptr,0u );

	// input (vertex) layout (2d position only)
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	GFX_THROW_INFO( pDevice->CreateInputLayout(
		ied,(UINT)std::size( ied ),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout
	) );

	// bind vertex layout
	pContext->IASetInputLayout( pInputLayout.Get() );

	// bind render target
	pContext->OMSetRenderTargets( 1u,pTarget.GetAddressOf(),nullptr );

	// Set primitive topology to triangle list (groups of 3 vertices)
	pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = 1280;
	vp.Height = 720;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports( 1u,&vp );

	GFX_THROW_INFO_ONLY( pContext->Draw( (UINT)std::size( vertices ),0u ) );
}

////////////////////////////////////////////////////////////////////////////////////

// Graphics exception stuff
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	: Exception(line, file), hr(hr)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Genix Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Genix Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

Graphics::InfoException::InfoException( int line,const char * file,std::vector<std::string> infoMsgs ) noexcept
	: Exception( line,file )
{
	// join all info messages with newlines into single string
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// remove final newline if exists
	if( !info.empty() )
	{
		info.pop_back();
	}
}

const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}