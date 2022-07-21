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

    IDXGISwapChain* SwapChain;
    ID3D11Texture2D* SwapchainBuffer;
    ID3D11RenderTargetView* SwapchainRenderTarget;
};

static d3d11_state State;

const D3D_DRIVER_TYPE DriverTypes[] =
{
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};

D3D11_BIND_FLAG BufferUsageToD3D11(rhi_buffer_usage Usage);

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

    RECT Rectangle;
    GetClientRect(Window, &Rectangle);
    State.Width = Rectangle.right - Rectangle.left;
    State.Height = Rectangle.bottom - Rectangle.top;

    VideoResize(State.Width, State.Height);
}

void VideoExit()
{
    SafeRelease(State.SwapchainRenderTarget);
    SafeRelease(State.SwapchainBuffer);
    SafeRelease(State.SwapChain);
    SafeRelease(State.Adapter);
    SafeRelease(State.DXGI);
    SafeRelease(State.DeviceContext);
    SafeRelease(State.Device);
}

void VideoResize(u32 Width, u32 Height)
{
    State.Width = Width;
    State.Height = Height;

    if (!State.SwapChain && State.Window) {
        DXGI_SWAP_CHAIN_DESC SwapchainCreateInfo = {};
        SwapchainCreateInfo.BufferDesc.Width = Width;
		SwapchainCreateInfo.BufferDesc.Height = Height;
		SwapchainCreateInfo.BufferDesc.RefreshRate.Numerator = 60; // TODO(milo): Get the refresh rate of the monitor
		SwapchainCreateInfo.BufferDesc.RefreshRate.Denominator = 1;
		SwapchainCreateInfo.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapchainCreateInfo.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		SwapchainCreateInfo.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		SwapchainCreateInfo.SampleDesc.Count = 1;		
		SwapchainCreateInfo.SampleDesc.Quality = 0;	
		SwapchainCreateInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapchainCreateInfo.BufferCount = 1;
		SwapchainCreateInfo.OutputWindow = State.Window;
		SwapchainCreateInfo.Flags = 0;
		SwapchainCreateInfo.Windowed = TRUE;

        HRESULT Result = State.Factory->CreateSwapChain(State.Device, &SwapchainCreateInfo, &State.SwapChain);
        if (FAILED(Result)) {
            LogCritical("Failed to create D3D11 swapchain!");
            return;
        }
    }

    SafeRelease(State.SwapchainRenderTarget);
    SafeRelease(State.SwapchainBuffer);

    State.SwapChain->ResizeBuffers(1, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    State.SwapChain->GetBuffer(0, IID_PPV_ARGS(&State.SwapchainBuffer));

    HRESULT Result = State.Device->CreateRenderTargetView(State.SwapchainBuffer, NULL, &State.SwapchainRenderTarget);
    if (FAILED(Result)) {
        LogCritical("Failed to create D3D11 swapchain render target view!");
        return;
    }
}

void VideoPresent()
{
    HRESULT Result = State.SwapChain->Present(1, 0);
    if (FAILED(Result)) {
        LogCritical("Failed to present D3D11 swapchain.");
    }
}

bool VideoReady()
{
    return State.SwapChain != NULL;
}

void BufferInit(rhi_buffer* Buffer, i64 Size, i64 Stride, rhi_buffer_usage Usage)
{
    Buffer->Stride = Stride;

    D3D11_BUFFER_DESC BufferCreateInfo = {};
    BufferCreateInfo.Usage = D3D11_USAGE_DEFAULT;
    BufferCreateInfo.ByteWidth = (UINT)Size;
    BufferCreateInfo.BindFlags = BufferUsageToD3D11(Usage);
    BufferCreateInfo.CPUAccessFlags = 0;
    BufferCreateInfo.MiscFlags = 0;

    HRESULT Result = State.Device->CreateBuffer(&BufferCreateInfo, NULL, (ID3D11Buffer**)&Buffer->Internal);
    if (FAILED(Result)) {
        LogError("Failed to create buffer!");
    }
}

void BufferFree(rhi_buffer* Buffer)
{
    ID3D11Buffer* Internal = (ID3D11Buffer*)Buffer->Internal;
    SafeRelease(Internal);
}

void BufferUpload(rhi_buffer* Buffer, void* Data)
{
    ID3D11Buffer* Internal = (ID3D11Buffer*)Buffer->Internal;
    State.DeviceContext->UpdateSubresource(Internal, NULL, NULL, Data, NULL, NULL);
}

void BufferBindVertex(rhi_buffer* Buffer)
{
    ID3D11Buffer* Internal = (ID3D11Buffer*)Buffer->Internal;

    u32 Stride = Buffer->Stride;
    u32 Offset = 0;
    State.DeviceContext->IASetVertexBuffers(0, 1, &Internal, &Stride, &Offset);
}

void BufferBindIndex(rhi_buffer* Buffer)
{
    ID3D11Buffer* Internal = (ID3D11Buffer*)Buffer->Internal;
    State.DeviceContext->IASetIndexBuffer(Internal, DXGI_FORMAT_R32_UINT, 0);
}

void BufferBindUniform(rhi_buffer* Buffer, i32 Binding, rhi_buffer_bind Bind)
{
    ID3D11Buffer* Internal = (ID3D11Buffer*)Buffer->Internal;
    switch (Bind)
    {
        case BufferBind_Vertex: {
            State.DeviceContext->VSGetConstantBuffers(Binding, 1, &Internal);
            break;
        }
        case BufferBind_Pixel: {
            State.DeviceContext->PSGetConstantBuffers(Binding, 1, &Internal);
            break;
        }
        case BufferBind_Compute: {
            State.DeviceContext->CSGetConstantBuffers(Binding, 1, &Internal);
            break;
        }
    }
}

D3D11_BIND_FLAG BufferUsageToD3D11(rhi_buffer_usage Usage)
{
    switch (Usage)
    {
        case BufferUsage_Vertex: {
            return D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
        }
        case BufferUsage_Index: {
            return D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
        }
        case BufferUsage_Uniform: {
            return D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
        }
        case BufferUsage_Storage: {
            return D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
        }
    }

    return D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
}

#endif