#pragma once

#include "backrooms.h"

#include <dr_libs/dr_wav.h>
#include <dr_libs/dr_mp3.h>
#include <dr_libs/dr_flac.h>

#define DEFAULT_AUDIO_CHANNELS 2
#define DEFAULT_AUDIO_SAMPLE_RATE 48000

enum audio_source_type
{
    AudioSourceType_FLAC,
    AudioSourceType_MP3,
    AudioSourceType_WAV
};

struct audio_source
{
    bool Looping;
    f32 Volume;
    f32 Pitch;
    i16* Samples;

    audio_source_type Type;
    struct {
        drwav Wave;
        drmp3 MP3;
        drflac* Flac;
    } Loaders;

    void* BackendData; // IXAudio2SourceVoice
};

void AudioInit();
void AudioExit();

void AudioSourceCreate(audio_source* Source);
void AudioSourceLoad(audio_source* Source, const char* Path, audio_source_type Type);
void AudioSourcePlay(audio_source* Source);
void AudioSourceStop(audio_source* Source);
void AudioSourceSetVolume(audio_source* Source, f32 Volume);
void AudioSourceSetPitch(audio_source* Source, f32 Pitch);
void AudioSourceSetLoop(audio_source* Source, bool Loop);
void AudioSourceDestroy(audio_source* Source);