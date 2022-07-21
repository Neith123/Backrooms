#pragma once

#include "backrooms_common.h"

enum rhi_buffer_usage
{
    BufferUsage_Vertex,
    BufferUsage_Index,
    BufferUsage_Uniform,
    BufferUsage_Storage
};

enum rhi_buffer_bind
{
    BufferBind_Vertex,
    BufferBind_Pixel,
    BufferBind_Compute
};

struct rhi_buffer
{
    void* Internal;
    i64 Stride;
};

void VideoInit(void* WindowHandle);
void VideoExit();
void VideoPresent();
void VideoResize(u32 Width, u32 Height);
bool VideoReady();

void BufferInit(rhi_buffer* Buffer, i64 Size, i64 Stride, rhi_buffer_usage Usage);
void BufferFree(rhi_buffer* Buffer);
void BufferUpload(rhi_buffer* Buffer, void* Data);
void BufferBindVertex(rhi_buffer* Buffer);
void BufferBindIndex(rhi_buffer* Buffer);
void BufferBindUniform(rhi_buffer* Buffer, i32 Binding, rhi_buffer_bind Bind);
// TODO(milo): Storage buffer