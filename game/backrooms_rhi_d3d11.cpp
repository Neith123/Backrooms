#include "backrooms_rhi.h"
#include "backrooms_logger.h"

#if defined(BACKROOMS_WINDOWS)

#include <d3d11.h>
#include <dxgi.h>

#define SafeRelease(ptr) if (ptr) ptr->Release()

struct d3d11_state
{
    HWND Window;
    u32 Width;
    u32 Height;

    IDXGIAdapter* Adapter;
    IDXGIFactory* Factory;
    IDXGIDevice* DXGI;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;
    D3D_FEATURE_LEVEL FeatureLevel;
};

static d3d11_state State;

const D3D_DRIVER_TYPE DriverTypes[] =
{
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};


void VideoInit(void* WindowHandle)
{
    HWND Window = (HWND)WindowHandle;
    State.Window = Window;

    u32 Flags = 0;
#if defined(_DEBUG)
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL Levels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT Result = 0;
    for (i32 DriverTypeIndex = 0; DriverTypeIndex < ARRAYSIZE(DriverTypes); DriverTypeIndex++) {
        Result = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, Flags, Levels, 1, D3D11_SDK_VERSION, &State.Device, &State.FeatureLevel, &State.DeviceContext);

        if (SUCCEEDED(Result))
            break;
    }

    if (FAILED(Result)) {
        LogCritical("Failed to initialise D3D11 device!");
        return;
    }

    State.Device->QueryInterface(IID_PPV_ARGS(&State.DXGI));
    State.DXGI->GetParent(IID_PPV_ARGS(&State.Adapter));
    State.Adapter->GetParent(IID_PPV_ARGS(&State.Factory));
}

void VideoExit()
{
    SafeRelease(State.Adapter);
    SafeRelease(State.DXGI);
    SafeRelease(State.DeviceContext);
    SafeRelease(State.Device);
}

#endif