#include "backrooms_rhi.h"
#include "backrooms_logger.h"
#include "backrooms_platform.h"

#if defined(BACKROOMS_WINDOWS)

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <string>
#include <vector>
#include <stb/stb_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

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

struct d3d11_texture
{
    ID3D11Texture2D* ColorTexture;

    ID3D11RenderTargetView* RTV;
    ID3D11DepthStencilView* DSV;
    ID3D11ShaderResourceView* SRV;
    ID3D11UnorderedAccessView* UAV;
};

struct d3d11_buffer
{
    ID3D11Buffer* Buffer;
    ID3D11ShaderResourceView* SRV;
    ID3D11UnorderedAccessView* UAV;
    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
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
D3D11_BIND_FLAG TextureUsageToD3D11(rhi_texture_usage Usage);

void VideoInit(void* WindowHandle)
{
    HWND Window = (HWND)WindowHandle;
    State.Window = Window;

    u32 Flags = 0;
#ifdef _DEBUG
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL Levels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT Result = 0;
    for (i32 DriverTypeIndex = 0; DriverTypeIndex < ARRAYSIZE(DriverTypes); DriverTypeIndex++) {
        Result = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, D3D11_CREATE_DEVICE_DEBUG, Levels, 1, D3D11_SDK_VERSION, &State.Device, &State.FeatureLevel, &State.DeviceContext);

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
    GetClientRect(State.Window, &Rectangle);
    AdjustWindowRect(&Rectangle, WS_OVERLAPPEDWINDOW, 0);
    State.Width = Rectangle.right - Rectangle.left;
    State.Height = Rectangle.bottom - Rectangle.top;

    VideoResize(State.Width, State.Height);

    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
	
    ImGui_ImplWin32_EnableDpiAwareness();
	ImGui_ImplWin32_Init(State.Window);
	ImGui_ImplDX11_Init(State.Device, State.DeviceContext);
}

void VideoExit()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

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

    State.DeviceContext->RSSetViewports(1, &Viewport);
    State.DeviceContext->OMSetRenderTargets(1, &State.SwapchainRenderTarget, NULL);
}

void VideoDraw(u32 Count, u32 Start)
{
    State.DeviceContext->Draw(Count, Start);
}

void VideoDrawIndexed(u32 Count, u32 Start)
{
    State.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    State.DeviceContext->DrawIndexed(Count, Start, 0);
}

void VideoDispatch(u32 X, u32 Y, u32 Z)
{
    State.DeviceContext->Dispatch(X, Y, Z);
}

void VideoBlitToSwapchain(rhi_texture* Texture)
{
    d3d11_texture* Internal = (d3d11_texture*)Texture->Internal;

    State.DeviceContext->CopyResource(State.SwapchainBuffer, Internal->ColorTexture);
}

void VideoImGuiBegin()
{   
    ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void VideoImGuiEnd()
{
    ImGuiIO& io = ImGui::GetIO();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void BufferInit(rhi_buffer* Buffer, i64 Size, i64 Stride, rhi_buffer_usage Usage)
{
    Buffer->Stride = Stride;
    Buffer->Internal = new d3d11_buffer();

    D3D11_BUFFER_DESC BufferCreateInfo = {};
    BufferCreateInfo.Usage = D3D11_USAGE_DEFAULT;
    BufferCreateInfo.ByteWidth = Size;
    BufferCreateInfo.BindFlags = BufferUsageToD3D11(Usage);
    BufferCreateInfo.CPUAccessFlags = 0;
    BufferCreateInfo.StructureByteStride = Stride;
    BufferCreateInfo.MiscFlags = 0;
    if (Usage == BufferUsage_Storage) {
        BufferCreateInfo.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    }

    HRESULT Result = State.Device->CreateBuffer(&BufferCreateInfo, NULL, (ID3D11Buffer**)&((d3d11_buffer*)Buffer->Internal)->Buffer);
    if (FAILED(Result)) {
        LogError("Failed to create buffer!");
    }
}

void BufferFree(rhi_buffer* Buffer)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;

    SafeRelease(Internal->SRV);
    SafeRelease(Internal->UAV);
    SafeRelease(Internal->Buffer);
    delete Buffer->Internal;
}

void BufferInitSRV(rhi_buffer* Buffer)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;

    D3D11_BUFFER_DESC Desc;
    Internal->Buffer->GetDesc(&Desc);

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    SRVDesc.BufferEx.FirstElement = 0;

    if (Desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
    {
        // This is a Raw Buffer

        SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        SRVDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
        SRVDesc.BufferEx.NumElements = Desc.ByteWidth / 4;
    } 
	else if (Desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
    {
        // This is a Structured Buffer

        SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
        SRVDesc.BufferEx.NumElements = Desc.ByteWidth / Desc.StructureByteStride;
    } 

    if (FAILED(State.Device->CreateShaderResourceView(Internal->Buffer, &SRVDesc, &Internal->SRV))) {
        LogCritical("Failed to create shader resource view!");
    }
}

void BufferInitUAV(rhi_buffer* Buffer)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;

    D3D11_BUFFER_DESC Desc;
    Internal->Buffer->GetDesc(&Desc);

    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    UAVDesc.Buffer.FirstElement = 0;

    if (Desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
    {
        // This is a Raw Buffer

        UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
        UAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        UAVDesc.Buffer.NumElements = Desc.ByteWidth / 4; 
    } 
	else if (Desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
    {
        // This is a Structured Buffer

        UAVDesc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
        UAVDesc.Buffer.NumElements = Desc.ByteWidth / Desc.StructureByteStride; 
    } 

    if (FAILED(State.Device->CreateUnorderedAccessView(Internal->Buffer, &UAVDesc, &Internal->UAV))) {
        LogCritical("Failed to create unordered access view!");
    }
}

void BufferUpload(rhi_buffer* Buffer, void* Data)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;

    State.DeviceContext->UpdateSubresource(Internal->Buffer, NULL, NULL, Data, NULL, NULL);
}

void BufferBindVertex(rhi_buffer* Buffer)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;

    u32 Stride = Buffer->Stride;
    u32 Offset = 0;
    State.DeviceContext->IASetVertexBuffers(0, 1, &Internal->Buffer, &Stride, &Offset);
}

void BufferBindIndex(rhi_buffer* Buffer)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;
    State.DeviceContext->IASetIndexBuffer(Internal->Buffer, DXGI_FORMAT_R32_UINT, 0);
}

void BufferBindUniform(rhi_buffer* Buffer, i32 Binding, rhi_uniform_bind Bind)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;

    switch (Bind)
    {
        case UniformBind_Vertex: {
            State.DeviceContext->VSSetConstantBuffers(Binding, 1, &Internal->Buffer);
            break;
        }
        case UniformBind_Pixel: {
            State.DeviceContext->PSSetConstantBuffers(Binding, 1, &Internal->Buffer);
            break;
        }
        case UniformBind_Compute: {
            State.DeviceContext->CSSetConstantBuffers(Binding, 1, &Internal->Buffer);
            break;
        }
    }
}

void BufferBindSRV(rhi_buffer* Buffer, i32 Binding)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;
    State.DeviceContext->CSSetShaderResources(Binding, 1, &Internal->SRV);
}

void BufferBindUAV(rhi_buffer* Buffer, i32 Binding)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;
    State.DeviceContext->CSSetUnorderedAccessViews(Binding, 1, &Internal->UAV, NULL);
}

void* BufferGetData(rhi_buffer* Buffer)
{
    d3d11_buffer* Internal = (d3d11_buffer*)Buffer->Internal;
    ID3D11Buffer* OutputBuffer = nullptr;

	D3D11_BUFFER_DESC Desc = {};
	Internal->Buffer->GetDesc(&Desc);
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Desc.Usage = D3D11_USAGE_STAGING;
    Desc.BindFlags = 0;
    Desc.MiscFlags = 0;

	if (SUCCEEDED(State.Device->CreateBuffer(&Desc, nullptr, &OutputBuffer)))
	{
		State.DeviceContext->CopyResource(OutputBuffer, Internal->Buffer);

		void* data;
		State.DeviceContext->Map(OutputBuffer, 0, D3D11_MAP_READ, 0, &Internal->MappedSubresource);
		data = Internal->MappedSubresource.pData;

		OutputBuffer->Release();

		return data;
	}
    
    return NULL;
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
        if (FAILED(State.Device->CreateComputeShader(CS->GetBufferPointer(), CS->GetBufferSize(), NULL, &Internal->CS))) {
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
    
    if (FAILED(State.Device->CreateSamplerState(&Desc, (ID3D11SamplerState**)&Sampler->Internal))) {
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

void ImageLoad(rhi_image* Image, const char* Path)
{   
    i32 Channels;
    Image->Data = stbi_load(Path, &Image->Width, &Image->Height, &Channels, STBI_rgb_alpha);
    Image->Float = false;
    if (!Image->Data)
        LogError("Failed to load image data: %s", Path);
}

void ImageLoadFloat(rhi_image* Image, const char* Path)
{
    i32 Channels;
    Image->Data = stbi_loadf(Path, &Image->Width, &Image->Height, &Channels, STBI_rgb_alpha);
    Image->Float = true;
    if (!Image->Data)
        LogError("Failed to load image data: %s", Path);
}

void ImageFree(rhi_image* Image)
{
    stbi_image_free(Image->Data);
}

void TextureInit(rhi_texture* Texture, i32 Width, i32 Height, rhi_texture_format Format, rhi_texture_usage Usage)
{
    Texture->Cube = false;
    Texture->Width = Width;
    Texture->Height = Height;
    Texture->Format = Format;
    Texture->Internal = new d3d11_texture();

    D3D11_TEXTURE2D_DESC Desc = {};
    Desc.Width = Width;
    Desc.Height = Height;
    Desc.Format = (DXGI_FORMAT)Format;
    Desc.ArraySize = 1;
    Desc.BindFlags = TextureUsageToD3D11(Usage);
    Desc.SampleDesc.Count = 1;
    Desc.MipLevels = 1;

    if (FAILED(State.Device->CreateTexture2D(&Desc, NULL, (ID3D11Texture2D**)&((d3d11_texture*)Texture->Internal)->ColorTexture))) {
        LogCritical("Failed to create texture!");
    }
}

void TextureInitCube(rhi_texture* Texture, i32 Width, i32 Height, rhi_texture_format Format, rhi_texture_usage Usage)
{
    Texture->Cube = true;
    Texture->Width = Width;
    Texture->Height = Height;
    Texture->Format = Format;
    Texture->Internal = new d3d11_texture();

    D3D11_TEXTURE2D_DESC Desc = {};
    Desc.Width = Width;
    Desc.Height = Height;
    Desc.Format = (DXGI_FORMAT)Format;
    Desc.ArraySize = 6;
    Desc.BindFlags = TextureUsageToD3D11(Usage);
    Desc.SampleDesc.Count = 1;
    Desc.MipLevels = 1;
    Desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    if (FAILED(State.Device->CreateTexture2D(&Desc, NULL, (ID3D11Texture2D**)&((d3d11_texture*)Texture->Internal)->ColorTexture))) {
        LogCritical("Failed to create texture!");
    }
}   

void TextureLoad(rhi_texture* Texture, const char* Path)
{
    Texture->Cube = false;
    Texture->Format = TextureFormat_R8G8B8A8_Unorm;
    Texture->Internal = new d3d11_texture();

    i32 Channels = 0;
    stbi_set_flip_vertically_on_load(true);
    u8* Buffer = stbi_load(Path, &Texture->Width, &Texture->Height, &Channels, STBI_rgb_alpha);
    if (!Buffer)
        LogCritical("Failed to load texture file: %s", Path);

    D3D11_TEXTURE2D_DESC Desc = {};
    Desc.Width = Texture->Width;
    Desc.Height = Texture->Height;
    Desc.Format = (DXGI_FORMAT)Texture->Format;
    Desc.ArraySize = 1;
    Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
    Desc.SampleDesc.Count = 1;
    Desc.MipLevels = 0;
    Desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    if (FAILED(State.Device->CreateTexture2D(&Desc, NULL, (ID3D11Texture2D**)&((d3d11_texture*)Texture->Internal)->ColorTexture))) {
        LogCritical("Failed to create texture!");
    }

    State.DeviceContext->UpdateSubresource(((d3d11_texture*)Texture->Internal)->ColorTexture, 0u, nullptr, Buffer, 4 * Texture->Width, 0u);

    TextureInitSRV(Texture, true);

    stbi_image_free(Buffer);
}

void TextureLoadFloat(rhi_texture* Texture, const char* Path)
{
    u32 ChannelSize = 4 * sizeof(f32);

    Texture->Cube = false;
    Texture->Format = TextureFormat_R32G32B32A32_Float;
    Texture->Internal = new d3d11_texture();

    i32 Channels = 0;
    stbi_set_flip_vertically_on_load(true);
    f32* Buffer = stbi_loadf(Path, &Texture->Width, &Texture->Height, &Channels, STBI_rgb_alpha);
    if (!Buffer)
        LogCritical("Failed to load texture file: %s", Path);

    D3D11_TEXTURE2D_DESC Desc = {};
    Desc.Width = Texture->Width;
    Desc.Height = Texture->Height;
    Desc.Format = (DXGI_FORMAT)Texture->Format;
    Desc.ArraySize = 1;
    Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    Desc.SampleDesc.Count = 1;
    Desc.MipLevels = 1;

    D3D11_SUBRESOURCE_DATA Subresource = {};
    Subresource.pSysMem = Buffer;
    Subresource.SysMemPitch = ChannelSize * Texture->Width;
    Subresource.SysMemSlicePitch = ChannelSize * Texture->Width * Texture->Height;

    if (FAILED(State.Device->CreateTexture2D(&Desc, &Subresource, (ID3D11Texture2D**)&((d3d11_texture*)Texture->Internal)->ColorTexture))) {
        LogCritical("Failed to create texture!");
    }

    TextureInitSRV(Texture, false);

    stbi_image_free(Buffer);
}

void TextureInitFromImage(rhi_texture* Texture, rhi_image* Image)
{
    u32 ChannelSize = 4 * sizeof(u8);

    Texture->Cube = false;
    Texture->Format = Image->Float ? TextureFormat_R32G32B32A32_Float : TextureFormat_R8G8B8A8_Unorm;
    Texture->Internal = new d3d11_texture();

    D3D11_TEXTURE2D_DESC Desc = {};
    Desc.Width = Image->Width;
    Desc.Height = Image->Height;
    Desc.Format = (DXGI_FORMAT)Texture->Format;
    Desc.ArraySize = 1;
    Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    Desc.SampleDesc.Count = 1;
    Desc.MipLevels = 1;

    D3D11_SUBRESOURCE_DATA Subresource = {};
    Subresource.pSysMem = Image->Data;
    Subresource.SysMemPitch = ChannelSize * Image->Width;
    Subresource.SysMemSlicePitch = ChannelSize * Image->Width * Image->Height;

    if (FAILED(State.Device->CreateTexture2D(&Desc, &Subresource, (ID3D11Texture2D**)&((d3d11_texture*)Texture->Internal)->ColorTexture))) {
        LogCritical("Failed to create texture!");
    }

    TextureInitSRV(Texture, Image->Float ? false : true);
}

void TextureFree(rhi_texture* Texture)
{
    SafeRelease(((d3d11_texture*)Texture->Internal)->UAV);
    SafeRelease(((d3d11_texture*)Texture->Internal)->DSV);
    SafeRelease(((d3d11_texture*)Texture->Internal)->SRV);
    SafeRelease(((d3d11_texture*)Texture->Internal)->RTV);
    SafeRelease(((d3d11_texture*)Texture->Internal)->ColorTexture);
    delete Texture->Internal;
}

void TextureInitRTV(rhi_texture* Texture)
{
    if (FAILED(State.Device->CreateRenderTargetView(((d3d11_texture*)Texture->Internal)->ColorTexture, NULL, &((d3d11_texture*)Texture->Internal)->RTV))) {
        LogCritical("Failed to create render target view!");
    }
}

void TextureInitDSV(rhi_texture* Texture)
{
    if (FAILED(State.Device->CreateDepthStencilView(((d3d11_texture*)Texture->Internal)->ColorTexture, NULL, &((d3d11_texture*)Texture->Internal)->DSV))) {
        LogCritical("Failed to create depth stencil view!");
    }
}

void TextureInitSRV(rhi_texture* Texture, bool Mips)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC Desc = {};
    Desc.Format = (DXGI_FORMAT)Texture->Format;
    Desc.ViewDimension = Texture->Cube ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
    Desc.Texture2D.MipLevels = Mips ? -1 : 1;
    Desc.Texture2D.MostDetailedMip = 0;

    if (FAILED(State.Device->CreateShaderResourceView(((d3d11_texture*)Texture->Internal)->ColorTexture, &Desc, &((d3d11_texture*)Texture->Internal)->SRV))) {
        LogCritical("Failed to create shader resource view!");
    }

    if (Mips) {
        State.DeviceContext->GenerateMips(((d3d11_texture*)Texture->Internal)->SRV);
    }
}

void TextureInitUAV(rhi_texture* Texture)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC Desc = {};
    Desc.Format = (DXGI_FORMAT)Texture->Format;
    Desc.ViewDimension = Texture->Cube ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;
    Desc.Texture2DArray.ArraySize = Texture->Cube ? 6 : 0;

    if (FAILED(State.Device->CreateUnorderedAccessView(((d3d11_texture*)Texture->Internal)->ColorTexture, &Desc, &((d3d11_texture*)Texture->Internal)->UAV))) {
        LogCritical("Failed to create unordered access view!");
    }
}

void TextureBindRTV(rhi_texture* Texture, rhi_texture* Depth, hmm_vec4 ClearColor)
{
    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = (FLOAT)State.Width;
    Viewport.Height = (FLOAT)State.Height;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    State.DeviceContext->RSSetViewports(1, &Viewport);

    d3d11_texture* Internal = (d3d11_texture*)Texture->Internal;

    ID3D11RenderTargetView* BindRTV = Internal->RTV;
    ID3D11DepthStencilView* BindDSV = nullptr;
    State.DeviceContext->ClearRenderTargetView(BindRTV, ClearColor.Elements);
    if (Depth) {
        BindDSV = ((d3d11_texture*)Depth->Internal)->DSV;
        State.DeviceContext->ClearDepthStencilView(BindDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    State.DeviceContext->OMSetRenderTargets(1, &BindRTV, BindDSV);
}

void TextureBindSRV(rhi_texture* Texture, i32 Binding, rhi_uniform_bind Bind)
{
    d3d11_texture* Internal = (d3d11_texture*)Texture->Internal;

    ID3D11ShaderResourceView* SRV[1] = { nullptr };
    SRV[0] = Internal->SRV;

    switch (Bind) {
        case UniformBind_Vertex: {
            State.DeviceContext->VSSetShaderResources(Binding, 1, SRV);
            return; 
        }
        case UniformBind_Pixel: {
            State.DeviceContext->PSSetShaderResources(Binding, 1, SRV);
            return; 
        }
        case UniformBind_Compute: {
            State.DeviceContext->CSSetShaderResources(Binding, 1, SRV);
            return;
        }
    }
}

void TextureBindUAV(rhi_texture* Texture, i32 Binding)
{
    d3d11_texture* Internal = (d3d11_texture*)Texture->Internal;

    State.DeviceContext->CSSetUnorderedAccessViews(Binding, 1, &Internal->UAV, NULL);
}

void TextureResetRTV()
{
    ID3D11RenderTargetView* const RTV[1] = { NULL };
	ID3D11DepthStencilView* DSV = NULL;

    State.DeviceContext->OMSetRenderTargets(1, RTV, DSV);
}

void TextureResetSRV(i32 Binding, rhi_uniform_bind Bind)
{
    ID3D11ShaderResourceView* const SRV[1] = { NULL };

    switch (Bind) {
        case UniformBind_Vertex: {
            State.DeviceContext->VSSetShaderResources(Binding, 1, SRV);
            return; 
        }
        case UniformBind_Pixel: {
            State.DeviceContext->PSSetShaderResources(Binding, 1, SRV);
            return; 
        }
        case UniformBind_Compute: {
            State.DeviceContext->CSSetShaderResources(Binding, 1, SRV);
            return;
        }
    }
}

void TextureResetUAV(i32 Binding)
{
    ID3D11UnorderedAccessView* const UAV[1] = { NULL };
    State.DeviceContext->CSSetUnorderedAccessViews(Binding, 1, UAV, NULL);
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
            return (D3D11_BIND_FLAG)(D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE);
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

D3D11_BIND_FLAG TextureUsageToD3D11(rhi_texture_usage Usage)
{
    switch (Usage) {
        case TextureUsage_RTV: {
            return D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
        }
        case TextureUsage_DSV: {
            return D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
        }
        case TextureUsage_SRV: {
            return D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
        }
        case TextureUsage_UAV: {
            return D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
        }
    }

    return D3D11_BIND_SHADER_RESOURCE;
}

#endif