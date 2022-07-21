#include "backrooms.h"

#include "backrooms_platform.h"
#include "backrooms_logger.h"
#include "backrooms_audio.h"
#include "backrooms_rhi.h"
#include "backrooms_camera.h"

struct scene_constant_buffer
{
    hmm_mat4 View;
    hmm_mat4 Projection;
};

struct game_state
{
    f32 LastFrame;

    audio_source TestSource;

    rhi_shader Shader;
    rhi_buffer Buffer;
    rhi_buffer IndexBuffer;
    rhi_buffer SceneBuffer;
    noclip_camera Camera;
};

static game_state State;

static float Vertices[] = {
    0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
   -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
};

static u32 Indices[] = {
    0, 1, 3,
    1, 2, 3
};  

void GameInit()
{
    CODE_BLOCK("Test Audio")
    {
        AudioSourceCreate(&State.TestSource);
        AudioSourceSetLoop(&State.TestSource, true);
        AudioSourceLoad(&State.TestSource, "data/sfx/SyncamoreTheme.wav", AudioSourceType_WAV);
        //AudioSourcePlay(&State.TestSource);
    }

    CODE_BLOCK("Pipeline assets")
    {
        ShaderInit(&State.Shader, "data/shaders/triangle/Vertex.hlsl", "data/shaders/triangle/Fragment.hlsl");
        ShaderBind(&State.Shader);

        i64 VertexStride = sizeof(float) * 6;
        BufferInit(&State.Buffer, sizeof(Vertices), VertexStride, BufferUsage_Vertex);
        BufferUpload(&State.Buffer, Vertices);

        BufferInit(&State.IndexBuffer, sizeof(Indices), 0, BufferUsage_Index);
        BufferUpload(&State.IndexBuffer, Indices);

        BufferInit(&State.SceneBuffer, sizeof(scene_constant_buffer), 0, BufferUsage_Uniform);

        NoClipCameraInit(&State.Camera);
    }

    LogInfo("Game initialised.");
}

void GameUpdate()
{
    f32 Time = PlatformTimerGet();
    f32 Delta = Time - State.LastFrame;
    State.LastFrame = Time;

    NoClipCameraInput(&State.Camera, Delta);
    NoClipCameraUpdate(&State.Camera, Delta);

    scene_constant_buffer ConstantBuffer;
    ConstantBuffer.Projection = State.Camera.Projection;
    ConstantBuffer.View = State.Camera.View;

    VideoBegin();
    ShaderBind(&State.Shader);
    BufferBindVertex(&State.Buffer);
    BufferBindIndex(&State.IndexBuffer);
    BufferUpload(&State.SceneBuffer, &ConstantBuffer);
    BufferBindUniform(&State.SceneBuffer, 0, BufferBind_Vertex);
    VideoDrawIndexed(6, 0);
}

void GameResize(i32 Width, i32 Height)
{
    NoClipCameraResize(&State.Camera, Width, Height);
}

void GameExit()
{
    BufferFree(&State.SceneBuffer);
    BufferFree(&State.IndexBuffer);
    BufferFree(&State.Buffer);
    ShaderFree(&State.Shader);

    AudioSourceStop(&State.TestSource);
    AudioSourceDestroy(&State.TestSource);

    LogInfo("Game shutdown.");
}