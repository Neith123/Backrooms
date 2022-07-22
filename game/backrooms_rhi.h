#pragma once

#include "backrooms_common.h"

#include <stdlib.h>

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

enum rhi_texture_bind
{
    TextureBind_RTV,
    TextureBind_DSV,
    TextureBind_SRV,
    TextureBind_UAV
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

// NOTE(milo): Video
void VideoInit(void* WindowHandle);
void VideoExit();
void VideoPresent();
void VideoResize(u32 Width, u32 Height);
bool VideoReady();
void VideoBegin();
void VideoDraw(u32 Count, u32 Start);
void VideoDrawIndexed(u32 Count, u32 Start);

// NOTE(milo): Buffer
void BufferInit(rhi_buffer* Buffer, i64 Size, i64 Stride, rhi_buffer_usage Usage);
void BufferFree(rhi_buffer* Buffer);
void BufferUpload(rhi_buffer* Buffer, void* Data);
void BufferBindVertex(rhi_buffer* Buffer);
void BufferBindIndex(rhi_buffer* Buffer);
void BufferBindUniform(rhi_buffer* Buffer, i32 Binding, rhi_uniform_bind Bind);

// NOTE(milo): Shader
void ShaderInit(rhi_shader* Shader, const char* V = NULL, const char* P = NULL, const char* C = NULL);
void ShaderFree(rhi_shader* Shader);
void ShaderBind(rhi_shader* Shader);

// NOTE(milo): Sampler
void SamplerInit(rhi_sampler* Sampler, rhi_sampler_address Address);
void SamplerFree(rhi_sampler* Sampler);
void SamplerBind(rhi_sampler* Sampler, i32 Binding, rhi_uniform_bind Bind);

// NOTE(milo): Material
void MaterialInit(rhi_material* Material, rhi_material_config Config);
void MaterialFree(rhi_material* Material);
void MaterialBind(rhi_material* Material);