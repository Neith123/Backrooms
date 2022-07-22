#pragma once

#include "backrooms_common.h"
#include "backrooms_model.h"
#include "backrooms_graph_types.h"
#include "backrooms_forward.h"

struct frame_graph
{
    forward_pass Forward;

    frame_graph_scene Scene;
};

void FrameGraphInit(frame_graph* Graph);
void FrameGraphUpdate(frame_graph* Graph);
void FrameGraphRender(frame_graph* Graph);
void FrameGraphResize(frame_graph* Graph, u32 Width, u32 Height);
void FrameGraphFree(frame_graph* Graph);