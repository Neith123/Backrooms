#pragma once

#include "backrooms_model.h"
#include "backrooms_rhi.h"
#include "backrooms_audio.h"

#include <vector>

#define MAX_SERIALISABLE_PATH 256
#define MAX_ENTITY_COUNT 1024

struct transform
{
    hmm_vec3 Translation;
    hmm_quaternion Rotation;
    hmm_vec3 Scale;

    bool IsDirty;
    hmm_mat4 Local;
};

struct entity
{
    // Transform
    transform Transform;

    // Mesh
    bool HasMesh;
    gpu_mesh Mesh;
    char MeshPath[MAX_SERIALISABLE_PATH];

    // Audio source
    bool HasAudio;
    audio_source Source;
    audio_source_type SourceType;
    char AudioPath[MAX_SERIALISABLE_PATH];
};

struct scene
{
    entity Entities[MAX_ENTITY_COUNT];
    u32 EntityCount;
};

//~ NOTE(milo): Transform
void TransformInit(transform* Transform);
void TransformSetPosition(transform* Transform, hmm_vec3 Translation);
void TransformSetRotation(transform* Transform, hmm_quaternion Rotation);
void TransformSetScale(transform* Transform, hmm_vec3 Scale);
void TransformUpdate(transform* Transform);

//~ NOTE(milo): Scene serialisation
void SerialiseScene(scene* Scene, const std::string& Path); 
void DeserialiseScene(scene* Scene, const std::string& Path);