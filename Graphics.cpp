#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include "GraphicsThrowMacros.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")
namespace wrl = Microsoft::WRL;

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
		struct
		{
			float x;
			float y;
		} pos;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
	};

	// create vertex buffer (1 2d triangle at center of screen)
	Vertex vertices[] =
	{
		{ 0.0f,0.5f,255,0,0,0 },
		{ 0.5f,-0.5f,0,255,0,0 },
		{ -0.5f,-0.5f,0,0,255,0 },
		{ -0.3f,0.3f,0,255,0,0 },
		{ 0.3f,0.3f,0,0,255,0 },
		{ 0.0f,-0.8f,255,0,0,0 },
	};
	vertices[0].color.g = 255;

	// Introduction to Buffers in Direct3D 11:
	// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-intro
	// ID3D11Buffer: A buffer interface accesses a buffer resource, 
	// which is unstructured memory. Buffers typically store vertex or index data.
	// There are three types of buffers: vertex, index, or a shader-constant buffer.
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //Identify how the buffer will be bound to the pipeline.
	bd.Usage = D3D11_USAGE_DEFAULT; //Identify how the buffer is expected to be read from and written to.
	bd.CPUAccessFlags = 0u; //0 if no CPU access is necessary
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices); //Size of the buffer in bytes.
	bd.StructureByteStride = sizeof( Vertex ); //The size of each element in the buffer structure (in bytes) when the buffer represents a structured buffer.
	
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices; // Pointer to the initialization data.

	// How to: Create a Vertex Buffer:
	// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-vertex-how-to
	//Creates a buffer (vertex buffer, index buffer, or shader-constant buffer).
	GFX_THROW_INFO( pDevice->CreateBuffer( 
						&bd, //A pointer to a D3D11_BUFFER_DESC structure that describes the buffer.
						&sd, //A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data; use NULL to allocate space only
						&pVertexBuffer //Address of a pointer to the ID3D11Buffer interface for the buffer object created. Set this parameter to NULL to validate the other input parameters 
					) );

	// Bind vertex buffer to pipeline
	const UINT stride = sizeof( Vertex );
	const UINT offset = 0u;
	// Bind an array of vertex buffers to the input-assembler stage.
	pContext->IASetVertexBuffers( 
		0u, //The first input slot for binding. The first vertex buffer is explicitly bound to the start slot
		1u, //The number of vertex buffers in the array.
		pVertexBuffer.GetAddressOf(),//A pointer to an array of vertex buffers
		&stride, //Pointer to an array of stride values; one stride value for each buffer in the vertex-buffer array.
		&offset //ointer to an array of offset values; one offset value for each buffer in the vertex-buffer array.
	);

	// create index buffer
	const unsigned short indices[] =
	{
		0,1,2,
		0,2,3,
		0,4,1,
		2,1,5,
	};
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	GFX_THROW_INFO(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	// bind index buffer
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);


	// This interface is used to return data of arbitrary length.
	// Blobs can be used as data buffers. Blobs can also be used for storing vertex, adjacency, 
	// and material information during mesh optimization, and for loading operations. 
	// Also, these objects are used to return object code and error messages 
	// in APIs that compile vertex, geometry, and pixel shaders.
	wrl::ComPtr<ID3DBlob> pBlob;
	
	/******************************** CREATE PIXEL SHADER ********************************/
	// A pixel-shader interface manages an executable program (a pixel shader) that controls the pixel-shader stage.
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	GFX_THROW_INFO(D3DReadFileToBlob(
		L"PixelShader.cso",// A pointer to a constant null-terminated string that contains the name of the file to read into memory.
		&pBlob  // A pointer to a variable that receives a pointer to the ID3DBlob interface 
				// that contains information that D3DReadFileToBlob read from the pFileName file.
	));

	GFX_THROW_INFO(pDevice->CreatePixelShader(
		pBlob->GetBufferPointer(), //A pointer to the compiled shader.
		pBlob->GetBufferSize(), // Size of the compiled pixel shader.
		nullptr, //A pointer to a class linkage interface (see ID3D11ClassLinkage); the value can be NULL.
		&pPixelShader //Address of a pointer to a ID3D11PixelShader interface.
	));
	
	/******************************** CREATE VERTEX SHADER ********************************/
	// A vertex-shader interface manages an executable program (a vertex shader) that controls the vertex-shader stage.
	// The vertex-shader interface has no methods; use HLSL to implement your shader functionality.
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	// Reads a file that is on disk into memory.
	GFX_THROW_INFO( D3DReadFileToBlob( 
						L"VertexShader.cso", // A pointer to a constant null-terminated string that contains the name of the file to read into memory.
						&pBlob // A pointer to a variable that receives a pointer to the ID3DBlob interface 
							   // that contains information that D3DReadFileToBlob read from the pFileName file.
					) );
	
	// Create a vertex-shader object from a compiled shader.
	GFX_THROW_INFO( pDevice->CreateVertexShader( 
						pBlob->GetBufferPointer(), // A pointer to the compiled shader.
						pBlob->GetBufferSize(), // Size of the compiled vertex shader.
						nullptr, // A pointer to a class linkage interface (see ID3D11ClassLinkage); the value can be NULL.
						&pVertexShader // Address of a pointer to a ID3D11VertexShader interface.
					) );

	/******************************** BIND VERTEX SHADER ********************************/
	// Set a vertex shader to the device.
	pContext->VSSetShader( 
		pVertexShader.Get(), // Pointer to a vertex shader (see ID3D11VertexShader). Passing in NULL disables the shader for this pipeline stage.
		nullptr, // A pointer to an array of class-instance interfaces. NULL if the shader does not use any interfaces.
		0u // The number of class-instance interfaces in the array.
	);



	/******************************** BIND PIXEL SHADER ********************************/
	pContext->PSSetShader( pPixelShader.Get(),nullptr,0u );

	// input (vertex) layout (2d position only)
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Color",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,8u,D3D11_INPUT_PER_VERTEX_DATA,0 },

	};
	GFX_THROW_INFO( pDevice->CreateInputLayout(
		ied,(UINT)std::size( ied ),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout
	) );

	/******************************** BIND VERTEX LAYOUT ********************************/
	pContext->IASetInputLayout( pInputLayout.Get() );

	/******************************** BIND RENDER TARGET ********************************/
	pContext->OMSetRenderTargets( 1u,pTarget.GetAddressOf(),nullptr );

	// Set primitive topology to triangle list (groups of 3 vertices)
	pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	/******************************** CONFIGURE VIEWPORT ********************************/
	// Defines the dimensions of a viewport.
	// In all cases, Width and Height must be >= 0 and 
	// TopLeftX + Width and TopLeftY + Height must be <= D3D11_VIEWPORT_BOUNDS_MAX.
	D3D11_VIEWPORT vp;
	vp.Width = 1280;
	vp.Height = 720;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	// Bind an array of viewports to the rasterizer stage of the pipeline.
	pContext->RSSetViewports( 
		1u, // Number of viewports to bind.
		&vp // An array of D3D11_VIEWPORT structures to bind to the device. 
	);

	// Draw: Draw non-indexed, non-instanced primitives.
	// Draw submits work to the rendering pipeline.
	// The vertex data for a draw call normally comes from a vertex buffer that is bound to the pipeline.
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u));
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