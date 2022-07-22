#include "backrooms_frame_graph.h"

void FrameGraphInit(frame_graph* Graph)
{
    BufferInit(&Graph->Scene.CameraBuffer, sizeof(frame_graph_camera_buffer), 0, BufferUsage_Uniform);

    ForwardPassInit(&Graph->Forward);
}

void FrameGraphUpdate(frame_graph* Graph)
{
    BufferUpload(&Graph->Scene.CameraBuffer, &Graph->Scene.Camera);
    ForwardPassRender(&Graph->Forward, &Graph->Scene);
}

void FrameGraphRender(frame_graph* Graph)
{
    VideoBlitToSwapchain(&Graph->Forward.Output);
}

void FrameGraphResize(frame_graph* Graph, u32 Width, u32 Height)
{
    ForwardPassResize(&Graph->Forward, Width, Height);
}

void FrameGraphFree(frame_graph* Graph)
{
    ForwardPassFree(&Graph->Forward);
    BufferFree(&Graph->Scene.CameraBuffer);
}