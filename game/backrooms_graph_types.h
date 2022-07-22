#pragma once

#include "backrooms_common.h"
#include "backrooms_model.h"

struct frame_graph_camera_buffer
{
    hmm_mat4 View;
    hmm_mat4 Projection;
};

// Temporary
struct frame_graph_scene
{
    std::vector<gpu_mesh> Meshes;

    frame_graph_camera_buffer Camera;
    rhi_buffer CameraBuffer;
};