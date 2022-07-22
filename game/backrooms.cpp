#include "backrooms.h"

#include "backrooms_platform.h"
#include "backrooms_logger.h"
#include "backrooms_audio.h"
#include "backrooms_rhi.h"
#include "backrooms_camera.h"
#include "backrooms_model.h"
#include "backrooms_frame_graph.h"

struct scene_constant_buffer
{
    hmm_mat4 View;
    hmm_mat4 Projection;
};

struct game_state
{
    f32 LastFrame;

    audio_source TestSource;

    frame_graph FrameGraph;
    gpu_mesh Helmet;
    noclip_camera Camera;
};

static game_state State;

void GameInit()
{
    CODE_BLOCK("Test Audio")
    {
        AudioSourceCreate(&State.TestSource);
        AudioSourceSetLoop(&State.TestSource, true);
        AudioSourceSetVolume(&State.TestSource, 0.3f);
        AudioSourceSetPitch(&State.TestSource, 0.9f);
        AudioSourceLoad(&State.TestSource, "data/sfx/ambiance0.mp3", AudioSourceType_MP3);
    }

    CODE_BLOCK("Pipeline assets")
    {
        FrameGraphInit(&State.FrameGraph);

        GpuMeshLoad(&State.Helmet, "data/models/DamagedHelmet.gltf");
        NoClipCameraInit(&State.Camera);

        State.FrameGraph.Scene.Meshes.push_back(State.Helmet);
    }

    AudioSourcePlay(&State.TestSource);

    LogInfo("Game initialised.");
}

void GameUpdate()
{
    f32 Time = PlatformTimerGet();
    f32 Delta = Time - State.LastFrame;
    State.LastFrame = Time;

    NoClipCameraInput(&State.Camera, Delta);
    NoClipCameraUpdate(&State.Camera, Delta);

    State.FrameGraph.Scene.Camera.Projection = State.Camera.Projection;
    State.FrameGraph.Scene.Camera.View = State.Camera.View;

    FrameGraphUpdate(&State.FrameGraph);
    FrameGraphRender(&State.FrameGraph);
}

void GameResize(i32 Width, i32 Height)
{
    NoClipCameraResize(&State.Camera, Width, Height);
    if (VideoReady()) {
        FrameGraphResize(&State.FrameGraph, (u32)Width, (u32)Height);
    }
}

void GameExit()
{
    GpuMeshFree(&State.Helmet);
    FrameGraphFree(&State.FrameGraph);

    AudioSourceStop(&State.TestSource);
    AudioSourceDestroy(&State.TestSource);

    LogInfo("Game shutdown.");
}