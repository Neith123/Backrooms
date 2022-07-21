#include "backrooms.h"

#include "backrooms_platform.h"
#include "backrooms_logger.h"
#include "backrooms_audio.h"
#include "backrooms_rhi.h"

struct game_state
{
    audio_source TestSource;

    rhi_shader TriangleShader;
    rhi_buffer TriangleBuffer;
    rhi_buffer TriangleIndex;
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
        AudioSourcePlay(&State.TestSource);
    }

    CODE_BLOCK("Pipeline assets")
    {
        ShaderInit(&State.TriangleShader, "data/shaders/triangle/Vertex.hlsl", "data/shaders/triangle/Fragment.hlsl");
        ShaderBind(&State.TriangleShader);

        i64 VertexStride = sizeof(float) * 6;
        BufferInit(&State.TriangleBuffer, sizeof(Vertices), VertexStride, BufferUsage_Vertex);
        BufferUpload(&State.TriangleBuffer, Vertices);

        BufferInit(&State.TriangleIndex, sizeof(Indices), 0, BufferUsage_Index);
        BufferUpload(&State.TriangleIndex, Indices);
    }

    LogInfo("Game initialised.");
}

void GameUpdate()
{
    VideoBegin();
    ShaderBind(&State.TriangleShader);
    BufferBindVertex(&State.TriangleBuffer);
    BufferBindIndex(&State.TriangleIndex);
    VideoDrawIndexed(6, 0);
}

void GameExit()
{
    BufferFree(&State.TriangleIndex);
    BufferFree(&State.TriangleBuffer);
    ShaderFree(&State.TriangleShader);

    AudioSourceStop(&State.TestSource);
    AudioSourceDestroy(&State.TestSource);

    LogInfo("Game shutdown.");
}