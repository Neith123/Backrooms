#pragma once

#include "backrooms_common.h"

#include <stdlib.h>
#include <hmm/HandmadeMath.h>

enum rhi_texture_format
{
    TextureFormat_Unknown = 0,
    TextureFormat_R32G32B32A32_Typeless = 1,
    TextureFormat_R32G32B32A32_Float = 2,
    TextureFormat_R32G32B32A32_Uint = 3,
    TextureFormat_R32G32B32A32_Sint = 4,
    TextureFormat_R32G32B32_Typeless = 5,
    TextureFormat_R32G32B32_Float = 6,
    TextureFormat_R32G32B32_Uint = 7,
    TextureFormat_R32G32B32_Sint = 8,
    TextureFormat_R16G16B16A16_Typeless = 9,
    TextureFormat_R16G16B16A16_Float = 10,
    TextureFormat_R16G16B16A16_Unorm = 11,
    TextureFormat_R16G16B16A16_Uint = 12,
    TextureFormat_R16G16B16A16_Snorm = 13,
    TextureFormat_R16G16B16A16_Sint = 14,
    TextureFormat_R32G32_Typeless = 15,
    TextureFormat_R32G32_Float = 16,
    TextureFormat_R32G32_Uint = 17,
    TextureFormat_R32G32_Sint = 18,
    TextureFormat_R32G8X24_Typeless = 19,
    TextureFormat_D32_Float_S8X24_Uint = 20,
    TextureFormat_R32_Float_X8X24_Typeless = 21,
    TextureFormat_X32_Typeless_G8X24_Uint = 22,
    TextureFormat_R10G10B10A2_Typeless = 23,
    TextureFormat_R10G10B10A2_Unorm = 24,
    TextureFormat_R10G10B10A2_Uint = 25,
    TextureFormat_R11G11B10_Float = 26,
    TextureFormat_R8G8B8A8_Typeless = 27,
    TextureFormat_R8G8B8A8_Unorm = 28,
    TextureFormat_R8G8B8A8_Unorm_SRGB = 29,
    TextureFormat_R8G8B8A8_Uint = 30,
    TextureFormat_R8G8B8A8_Snorm = 31,
    TextureFormat_R8G8B8A8_Sint = 32,
    TextureFormat_R16G16_Typeless = 33,
    TextureFormat_R16G16_Float = 34,
    TextureFormat_R16G16_Unorm = 35,
    TextureFormat_R16G16_Uint = 36,
    TextureFormat_R16G16_Snorm = 37,
    TextureFormat_R16G16_Sint = 38,
    TextureFormat_R32_Typeless = 39,
    TextureFormat_D32_Float = 40,
    TextureFormat_R32_Float = 41,
    TextureFormat_R32_Uint = 42,
    TextureFormat_R32_Sint = 43,
    TextureFormat_R24G8_Typeless = 44,
    TextureFormat_D24_Unorm_S8_Uint = 45,
    TextureFormat_R24_Unorm_X8_Typeless = 46,
    TextureFormat_X24_Typeless_G8_Uint = 47,
    TextureFormat_R8G8_Typeless = 48,
    TextureFormat_R8G8_Unorm = 49,
    TextureFormat_R8G8_Uint = 50,
    TextureFormat_R8G8_Snorm = 51,
    TextureFormat_R8G8_Sint = 52,
    TextureFormat_R16_Typeless = 53,
    TextureFormat_R16_Float = 54,
    TextureFormat_D16_Unorm = 55,
    TextureFormat_R16_Unorm = 56,
    TextureFormat_R16_Uint = 57,
    TextureFormat_R16_Snorm = 58,
    TextureFormat_R16_Sint = 59,
    TextureFormat_R8_Typeless = 60,
    TextureFormat_R8_Unorm = 61,
    TextureFormat_R8_Uint = 62,
    TextureFormat_R8_Snorm = 63,
    TextureFormat_R8_Sint = 64,
    TextureFormat_A8_Unorm = 65,
    TextureFormat_R1_Unorm = 66,
    TextureFormat_R9G9B9E5_SHAREDEXP = 67,
    TextureFormat_R8G8_B8G8_Unorm = 68,
    TextureFormat_G8R8_G8B8_Unorm = 69,
    TextureFormat_BC1_Typeless = 70,
    TextureFormat_BC1_Unorm = 71,
    TextureFormat_BC1_Unorm_SRGB = 72,
    TextureFormat_BC2_Typeless = 73,
    TextureFormat_BC2_Unorm = 74,
    TextureFormat_BC2_Unorm_SRGB = 75,
    TextureFormat_BC3_Typeless = 76,
    TextureFormat_BC3_Unorm = 77,
    TextureFormat_BC3_Unorm_SRGB = 78,
    TextureFormat_BC4_Typeless = 79,
    TextureFormat_BC4_Unorm = 80,
    TextureFormat_BC4_Snorm = 81,
    TextureFormat_BC5_Typeless = 82,
    TextureFormat_BC5_Unorm = 83,
    TextureFormat_BC5_Snorm = 84,
    TextureFormat_B5G6R5_Unorm = 85,
    TextureFormat_B5G5R5A1_Unorm = 86,
    TextureFormat_B8G8R8A8_Unorm = 87,
    TextureFormat_B8G8R8X8_Unorm = 88,
    TextureFormat_R10G10B10_XR_BIAS_A2_Unorm = 89,
    TextureFormat_B8G8R8A8_Typeless = 90,
    TextureFormat_B8G8R8A8_Unorm_SRGB = 91,
    TextureFormat_B8G8R8X8_Typeless = 92,
    TextureFormat_B8G8R8X8_Unorm_SRGB = 93,
    TextureFormat_BC6H_Typeless = 94,
    TextureFormat_BC6H_UF16 = 95,
    TextureFormat_BC6H_SF16 = 96,
    TextureFormat_BC7_Typeless = 97,
    TextureFormat_BC7_Unorm = 98,
    TextureFormat_BC7_Unorm_SRGB = 99,
    TextureFormat_AYUV = 100,
    TextureFormat_Y410 = 101,
    TextureFormat_Y416 = 102,
    TextureFormat_NV12 = 103,
    TextureFormat_P010 = 104,
    TextureFormat_P016 = 105,
    TextureFormat_420_OPAQUE = 106,
    TextureFormat_YUY2 = 107,
    TextureFormat_Y210 = 108,
    TextureFormat_Y216 = 109,
    TextureFormat_NV11 = 110,
    TextureFormat_AI44 = 111,
    TextureFormat_IA44 = 112,
    TextureFormat_P8 = 113,
    TextureFormat_A8P8 = 114,
    TextureFormat_B4G4R4A4_Unorm = 115,
    TextureFormat_P208 = 130,
    TextureFormat_V208 = 131,
    TextureFormat_V408 = 132,
};

enum rhi_buffer_usage
{
    BufferUsage_Vertex,
    BufferUsage_Index,
    BufferUsage_Uniform,
    BufferUsage_Storage
};

enum rhi_uniform_bind
{
    UniformBind_Vertex,
    UniformBind_Pixel,
    UniformBind_Compute
};

enum rhi_fill_mode
{
    FillMode_Fill,
    FillMode_Line
};

enum rhi_cull_mode
{
    CullMode_None,
    CullMode_Front,
    CullMode_Back
};

enum rhi_comp_op
{
    CompareOP_Never,
    CompareOP_Less,
    CompareOP_Equal,
    CompareOP_LessEqual,
    CompareOP_Greater,
    CompareOP_NotEqual,
    CompareOP_GreaterEqual,
    CompareOP_Always
};

enum rhi_texture_usage
{
    TextureUsage_RTV,
    TextureUsage_DSV,
    TextureUsage_SRV,
    TextureUsage_UAV
};

enum rhi_sampler_address
{
    SamplerAddress_Wrap,
    SamplerAddress_Mirror,
    SamplerAddress_Clamp,
    SamplerAddress_Border
};  

struct rhi_sampler
{
    rhi_sampler_address Address;
    void* Internal;
};

struct rhi_image
{
    void* Data;
    i32 Width;
    i32 Height;
    bool Float;
};

struct rhi_texture
{
    void* Internal;

    rhi_texture_format Format;
    i32 Width, Height;
    bool Cube;
};

struct rhi_material_config
{
    rhi_fill_mode FillMode;
    rhi_cull_mode CullMode;
    rhi_comp_op CompareOP;
    bool FrontFaceCCW;
};

struct rhi_material
{
    void* Internal;
    rhi_material_config Config;
};  

struct rhi_buffer
{
    void* Internal;
    i64 Stride;
};

struct rhi_shader
{
    void* Internal;
};

//~ NOTE(milo): Video
void VideoInit(void* WindowHandle);
void VideoExit();
void VideoPresent();
void VideoResize(u32 Width, u32 Height);
bool VideoReady();
void VideoBegin();
void VideoDraw(u32 Count, u32 Start);
void VideoDrawIndexed(u32 Count, u32 Start);
void VideoDispatch(u32 X, u32 Y, u32 Z);
void VideoBlitToSwapchain(rhi_texture* Texture);
void VideoImGuiBegin();
void VideoImGuiEnd();

//~ NOTE(milo): Buffer
void BufferInit(rhi_buffer* Buffer, i64 Size, i64 Stride, rhi_buffer_usage Usage);
void BufferFree(rhi_buffer* Buffer);
void BufferInitSRV(rhi_buffer* Buffer);
void BufferInitUAV(rhi_buffer* Buffer);
void BufferUpload(rhi_buffer* Buffer, void* Data);
void BufferBindVertex(rhi_buffer* Buffer);
void BufferBindIndex(rhi_buffer* Buffer);
void BufferBindUniform(rhi_buffer* Buffer, i32 Binding, rhi_uniform_bind Bind);
void BufferBindSRV(rhi_buffer* Buffer, i32 Binding);
void BufferBindUAV(rhi_buffer* Buffer, i32 Binding);
void* BufferGetData(rhi_buffer* Buffer);

//~ NOTE(milo): Shader
void ShaderInit(rhi_shader* Shader, const char* V = NULL, const char* P = NULL, const char* C = NULL);
void ShaderFree(rhi_shader* Shader);
void ShaderBind(rhi_shader* Shader);

//~ NOTE(milo): Sampler
void SamplerInit(rhi_sampler* Sampler, rhi_sampler_address Address);
void SamplerFree(rhi_sampler* Sampler);
void SamplerBind(rhi_sampler* Sampler, i32 Binding, rhi_uniform_bind Bind);

//~ NOTE(milo): Image
void ImageLoad(rhi_image* Image, const char* Path);
void ImageLoadFloat(rhi_image* Image, const char* Path);
void ImageFree(rhi_image* Image);

//~ NOTE(milo): Texture
void TextureInit(rhi_texture* Texture, i32 Width, i32 Height, rhi_texture_format Format, rhi_texture_usage Usage);
void TextureInitCube(rhi_texture* Texture, i32 Width, i32 Height, rhi_texture_format Format, rhi_texture_usage Usage);
void TextureLoad(rhi_texture* Texture, const char* Path); // NOTE(milo): Deprecated
void TextureLoadFloat(rhi_texture* Texture, const char* Path); // NOTE(milo): Deprecated
void TextureInitFromImage(rhi_texture* Texture, rhi_image* Image);
void TextureFree(rhi_texture* Texture);
void TextureInitRTV(rhi_texture* Texture);
void TextureInitDSV(rhi_texture* Texture);
void TextureInitSRV(rhi_texture* Texture, bool Mips);
void TextureInitUAV(rhi_texture* Texture);
void TextureBindRTV(rhi_texture* Texture, rhi_texture* Depth, hmm_vec4 ClearColor);
void TextureBindSRV(rhi_texture* Texture, i32 Binding, rhi_uniform_bind Bind);
void TextureBindUAV(rhi_texture* Texture, i32 Binding);
void TextureResetRTV();
void TextureResetSRV(i32 Binding, rhi_uniform_bind Bind);
void TextureResetUAV(i32 Binding);

//~ NOTE(milo): Material
void MaterialInit(rhi_material* Material, rhi_material_config Config);
void MaterialFree(rhi_material* Material);
void MaterialBind(rhi_material* Material);