#include "backrooms.h"

#include "backrooms_platform.h"
#include "backrooms_logger.h"
#include "backrooms_audio.h"

struct game_state
{
    audio_source TestSource;
};

static game_state State;

void GameInit()
{
    LogInfo("Game initialised.");

    AudioSourceCreate(&State.TestSource);
    AudioSourceSetLoop(&State.TestSource, true);
    AudioSourceLoad(&State.TestSource, "data/sfx/SyncamoreTheme.wav", AudioSourceType_WAV);
    AudioSourcePlay(&State.TestSource);
}

void GameUpdate()
{

}

void GameExit()
{
    AudioSourceDestroy(&State.TestSource);

    LogInfo("Game shutdown.");
}