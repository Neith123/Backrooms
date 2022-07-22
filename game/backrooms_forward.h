#pragma once 

#include "backrooms_rhi.h"

#include "backrooms_graph_types.h"

struct forward_pass
{
    rhi_texture Output;
    rhi_texture Depth;

    rhi_shader ForwardShader;
    rhi_material ForwardMaterial;
    rhi_sampler ForwardSampler;
};

void ForwardPassInit(forward_pass* Pass);
void ForwardPassRender(forward_pass* Pass, frame_graph_scene* Scene);
void ForwardPassResize(forward_pass* Pass, u32 Width, u32 Height);
void ForwardPassFree(forward_pass* Pass);