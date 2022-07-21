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
    AudioSourceCreate(&State.TestSource);
    AudioSourceSetLoop(&State.TestSource, true);
    AudioSourceLoad(&State.TestSource, "data/sfx/SyncamoreTheme.wav", AudioSourceType_WAV);
    AudioSourcePlay(&State.TestSource);

    LogInfo("Game initialised.");
}

void GameUpdate()
{

}

void GameExit()
{
    AudioSourceStop(&State.TestSource);
    AudioSourceDestroy(&State.TestSource);

    LogInfo("Game shutdown.");
}