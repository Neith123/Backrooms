#include "backrooms_rhi.h"
#include "backrooms_logger.h"
#include "backrooms_platform.h"

#if defined(BACKROOMS_WINDOWS)

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <string>
#include <vector>

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

struct d3d11_shader
{
    ID3D11VertexShader* VS;
    ID3D11PixelShader* PS;
    ID3D11ComputeShader* CS;
    ID3D11InputLayout* InputLayout;  
};

struct d3d11_material
{
    ID3D11RasterizerState* RState;
    ID3D11DepthStencilState* DState;
};

static d3d11_state State;

const D3D_DRIVER_TYPE DriverTypes[] =
{
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};

D3D11_BIND_FLAG BufferUsageToD3D11(rhi_buffer_usage Usage);
D3D11_CULL_MODE CullModeToD3D11(rhi_cull_mode Cull);
D3D11_FILL_MODE FillModeToD3D11(rhi_fill_mode Fill);
D3D11_COMPARISON_FUNC CompareToD3D11(rhi_comp_op Compare);
D3D11_TEXTURE_ADDRESS_MODE SamplerAddressToD3D11(rhi_sampler_address Address);

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

void VideoBegin()
{
    D3D11_VIEWPORT Viewport;
    ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = (FLOAT)State.Width;
    Viewport.Height = (FLOAT)State.Height;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;

    FLOAT Clear[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    State.DeviceContext->RSSetViewports(1, &Viewport);
    State.DeviceContext->ClearRenderTargetView(State.SwapchainRenderTarget, Clear);
    State.DeviceContext->OMSetRenderTargets(1, &State.SwapchainRenderTarget, NULL);
}

void VideoDraw(u32 Count, u32 Start)
{
    State.DeviceContext->Draw(Count, Start);
}

void VideoDrawIndexed(u32 Count, u32 Start)
{
    State.DeviceContext->DrawIndexed(Count, Start, 0);
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

void BufferBindUniform(rhi_buffer* Buffer, i32 Binding, rhi_uniform_bind Bind)
{
    ID3D11Buffer* Internal = (ID3D11Buffer*)Buffer->Internal;
    switch (Bind)
    {
        case UniformBind_Vertex: {
            State.DeviceContext->VSSetConstantBuffers(Binding, 1, &Internal);
            break;
        }
        case UniformBind_Pixel: {
            State.DeviceContext->PSSetConstantBuffers(Binding, 1, &Internal);
            break;
        }
        case UniformBind_Compute: {
            State.DeviceContext->CSSetConstantBuffers(Binding, 1, &Internal);
            break;
        }
    }
}

ID3DBlob* CompileBlob(std::string Source, const char* Profile)
{
    ID3DBlob* ShaderBlob;
    ID3DBlob* ErrorBlob;
    HRESULT Status = D3DCompile(Source.c_str(), Source.size(), NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", Profile, 0, 0, &ShaderBlob, &ErrorBlob);
    if (ErrorBlob)
        LogCritical("Shader Error (profile: %s) : %s", Profile, (char*)ErrorBlob->GetBufferPointer());
    return ShaderBlob;
}

void ShaderInit(rhi_shader* Shader, const char* V, const char* P, const char* C)
{
    Shader->Internal = new d3d11_shader;
    d3d11_shader* Internal = (d3d11_shader*)Shader->Internal;
    ZeroMemory(Internal, sizeof(d3d11_shader));

    ID3DBlob* VS = nullptr;
    ID3DBlob* PS = nullptr;
    ID3DBlob* CS = nullptr;

    if (V) {
        VS = CompileBlob(PlatformReadFile(V), "vs_5_0");
        if (FAILED(State.Device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &Internal->VS))) {
            LogCritical("Failed to create vertex shader!");
        }
    }
    if (P) {
        PS = CompileBlob(PlatformReadFile(P), "ps_5_0");
        if (FAILED(State.Device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &Internal->PS))) {
            LogCritical("Failed to create pixel shader!");
        }
    }
    if (C) {
        CS = CompileBlob(PlatformReadFile(C), "cs_5_0");
        if (FAILED(State.Device->CreateComputeShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &Internal->CS))) {
            LogCritical("Failed to create compute shader!");
        }
    }

    if (VS) {
        ID3D11ShaderReflection* Reflect = NULL;
        if (FAILED(D3DReflect(VS->GetBufferPointer(), VS->GetBufferSize(), IID_PPV_ARGS(&Reflect)))) {
            LogCritical("Failed to reflect vertex shader!");
        }

        D3D11_SHADER_DESC ShaderDesc;
        Reflect->GetDesc(&ShaderDesc);

        std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutDesc;
        for (u32 ParameterIndex = 0; ParameterIndex < ShaderDesc.InputParameters; ParameterIndex++)
        {
            D3D11_SIGNATURE_PARAMETER_DESC ParamDesc;
            Reflect->GetInputParameterDesc(ParameterIndex, &ParamDesc);

            D3D11_INPUT_ELEMENT_DESC ElementDesc;
            ElementDesc.SemanticName = ParamDesc.SemanticName;
            ElementDesc.SemanticIndex = ParamDesc.SemanticIndex;
            ElementDesc.InputSlot = 0;
            ElementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            ElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            ElementDesc.InstanceDataStepRate = 0;   

            if (ParamDesc.Mask == 1)
            {
                if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32_UINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32_SINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32_FLOAT;
            }
            else if (ParamDesc.Mask <= 3)
            {
                if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32G32_UINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32G32_SINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
            }
            else if ( ParamDesc.Mask <= 7 )
            {
                if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            }
            else if (ParamDesc.Mask <= 15)
            {
                if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                else if (ParamDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }

            InputLayoutDesc.push_back(ElementDesc);
        }

        if (FAILED(State.Device->CreateInputLayout(&InputLayoutDesc[0], (UINT)InputLayoutDesc.size(), VS->GetBufferPointer(), VS->GetBufferSize(), &Internal->InputLayout))) {
            LogCritical("Failed to create input layout!");
        }

        SafeRelease(Reflect);
    }

    SafeRelease(CS);
    SafeRelease(PS);
    SafeRelease(VS);
}

void ShaderFree(rhi_shader* Shader)
{
    d3d11_shader* Internal = (d3d11_shader*)Shader->Internal;
    SafeRelease(Internal->InputLayout);
    SafeRelease(Internal->CS);
    SafeRelease(Internal->PS);
    SafeRelease(Internal->VS);

    delete Shader->Internal;
}

void ShaderBind(rhi_shader* Shader)
{
    d3d11_shader* Internal = (d3d11_shader*)Shader->Internal;
    if (Internal->VS) State.DeviceContext->VSSetShader(Internal->VS, NULL, 0);
    if (Internal->PS) State.DeviceContext->PSSetShader(Internal->PS, NULL, 0);
    if (Internal->CS) State.DeviceContext->CSSetShader(Internal->CS, NULL, 0);
    if (Internal->InputLayout) State.DeviceContext->IASetInputLayout(Internal->InputLayout);
}

void SamplerInit(rhi_sampler* Sampler, rhi_sampler_address Address)
{
    Sampler->Address = Address;

    D3D11_SAMPLER_DESC Desc = {};
    Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    Desc.AddressU = SamplerAddressToD3D11(Address);
    Desc.AddressV = Desc.AddressU;
    Desc.AddressW = Desc.AddressU;
    Desc.MipLODBias = 0.0f;
    Desc.MaxAnisotropy = 1;
    Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    Desc.MinLOD = 0.0f;
    Desc.MaxLOD = D3D11_FLOAT32_MAX;
    
    if (FAILED(State.Device->CreateSamplerState(&Desc, (ID3D11SamplerState**)Sampler->Internal))) {
        LogCritical("Failed to create sampler state!");
    }
}

void SamplerFree(rhi_sampler* Sampler)
{
    ((ID3D11SamplerState*)Sampler->Internal)->Release();
}

void SamplerBind(rhi_sampler* Sampler, i32 Binding, rhi_uniform_bind Bind)
{
    switch (Bind) {
        case UniformBind_Vertex: {
            State.DeviceContext->VSSetSamplers(Binding, 1, (ID3D11SamplerState**)&Sampler->Internal);
            break;
        }
        case UniformBind_Pixel: {
            State.DeviceContext->PSSetSamplers(Binding, 1, (ID3D11SamplerState**)&Sampler->Internal);
            break;
        }
        case UniformBind_Compute: {
            State.DeviceContext->CSSetSamplers(Binding, 1, (ID3D11SamplerState**)&Sampler->Internal);
            break;
        }
    }
}

void MaterialInit(rhi_material* Material, rhi_material_config Config)
{
    Material->Config = Config;
    Material->Internal = new d3d11_material;
    d3d11_material* Internal = (d3d11_material*)Material->Internal;
    ZeroMemory(Internal, sizeof(d3d11_material));

    D3D11_RASTERIZER_DESC Desc = {};
    Desc.CullMode = CullModeToD3D11(Config.CullMode);
    Desc.FillMode = FillModeToD3D11(Config.FillMode);
    Desc.FrontCounterClockwise = (BOOL)Config.FrontFaceCCW;

    if (FAILED(State.Device->CreateRasterizerState(&Desc, &Internal->RState))) {
        LogCritical("Failed to create material rasterizer state!");
    }

    D3D11_DEPTH_STENCIL_DESC DDesc = {};
    DDesc.DepthEnable = true;
    DDesc.DepthFunc = CompareToD3D11(Config.CompareOP);
    DDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

    if (FAILED(State.Device->CreateDepthStencilState(&DDesc, &Internal->DState))) {
        LogCritical("Failed to create material depth stencil state!");
    }
}

void MaterialFree(rhi_material* Material)
{
    d3d11_material* Internal = (d3d11_material*)Material->Internal;
    SafeRelease(Internal->DState);
    SafeRelease(Internal->RState);
    delete Internal;
}

void MaterialBind(rhi_material* Material)
{
    d3d11_material* Internal = (d3d11_material*)Material->Internal;

    State.DeviceContext->RSSetState(Internal->RState);
    State.DeviceContext->OMSetDepthStencilState(Internal->DState, 0);
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

D3D11_CULL_MODE CullModeToD3D11(rhi_cull_mode Cull)
{
    switch (Cull)
    {
        case CullMode_None: {
            return D3D11_CULL_MODE::D3D11_CULL_NONE;
        }
        case CullMode_Back: {
            return D3D11_CULL_MODE::D3D11_CULL_BACK;
        }
        case CullMode_Front: {
            return D3D11_CULL_MODE::D3D11_CULL_FRONT;
        }
    }
    
    return D3D11_CULL_MODE::D3D11_CULL_NONE;
}

D3D11_FILL_MODE FillModeToD3D11(rhi_fill_mode Fill)
{
    switch (Fill)
    {
        case FillMode_Fill: {
            return D3D11_FILL_MODE::D3D11_FILL_SOLID;
        }
        case FillMode_Line: {
            return D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
        }
    }

    return D3D11_FILL_MODE::D3D11_FILL_SOLID;
}

D3D11_COMPARISON_FUNC CompareToD3D11(rhi_comp_op Compare)
{
    switch (Compare)
    {
        case CompareOP_Always: {
            return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
        }
        case CompareOP_Never: {
            return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
        }
        case CompareOP_Less: {
            return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
        }
        case CompareOP_Greater: {
            return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER;
        }
        case CompareOP_LessEqual: {
            return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
        }
        case CompareOP_GreaterEqual: {
            return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER_EQUAL;
        }
        case CompareOP_NotEqual: {
            return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL;
        }
    }

    return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
}

D3D11_TEXTURE_ADDRESS_MODE SamplerAddressToD3D11(rhi_sampler_address Address)
{
    switch (Address) {
        case SamplerAddress_Border: {
            return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
        }
        case SamplerAddress_Clamp: {
            return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
        }
        case SamplerAddress_Mirror: {
            return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR;
        }
        case SamplerAddress_Wrap: {
            return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
        }
    }

    return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
}

#endif