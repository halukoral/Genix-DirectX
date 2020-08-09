#pragma once

#define GHWND_EXCEPT( hr ) Window::HrException( __LINE__,__FILE__,(hr) )
#define GHWND_LAST_EXCEPT() Window::HrException( __LINE__,__FILE__,GetLastError() )
#define GHWND_NOGFX_EXCEPT() Window::NoGfxException( __LINE__,__FILE__ )