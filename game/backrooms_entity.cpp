#include "backrooms_entity.h"
#include "backrooms_logger.h"

#include <stdio.h>

struct entity_serialisation_data
{
    hmm_vec3 Translation;
    hmm_quaternion Rotation;
    hmm_vec3 Scale;

    bool HasMesh;
    char MeshPath[MAX_SERIALISABLE_PATH];

    bool HasAudio;
    audio_source_type SourceType;
    bool Looping;
    f32 Volume;
    f32 Pitch;
    char AudioPath[MAX_SERIALISABLE_PATH];
};

void TransformInit(transform* Transform)
{
    Transform->IsDirty = false;
    Transform->Translation = HMM_Vec3(0.0f, 0.0f, 0.0f);
    Transform->Rotation = HMM_Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    Transform->Scale = HMM_Vec3(1.0f, 1.0f, 1.0f);
    Transform->Local = HMM_Mat4d(1.0f);
}

void TransformSetPosition(transform* Transform, hmm_vec3 Translation)
{
    Transform->Translation = Translation;
}

void TransformSetRotation(transform* Transform, hmm_quaternion Rotation)
{
    Transform->Rotation = Rotation;
}

void TransformSetScale(transform* Transform, hmm_vec3 Scale)
{
    Transform->Scale = Scale;
}

void TransformUpdate(transform* Transform)
{
    if (Transform->IsDirty) {
        Transform->Local = HMM_MultiplyMat4(HMM_Translate(Transform->Translation), HMM_MultiplyMat4(HMM_QuaternionToMat4(Transform->Rotation), HMM_Scale(Transform->Scale)));
        Transform->IsDirty = false;
    }
}

void SerialiseScene(scene* Scene, const std::string& Path)
{
    FILE* File = fopen(Path.c_str(), "wb+");
    if (!File) {
        LogError("Failed to create serialisable scene file: %s", Path.c_str());
        return;
    }

    fwrite(&Scene->EntityCount, sizeof(u32), 1, File);
    for (i32 EntityIndex = 0; EntityIndex < Scene->EntityCount; EntityIndex++) {
        entity* Entity = &Scene->Entities[EntityIndex];

        // 12 bytes: translation
        fwrite(Entity->Transform.Translation.Elements, sizeof(f32), 3, File);
        // 16 bytes: rotation
        fwrite(Entity->Transform.Rotation.Elements, sizeof(f32), 4 , File);
        // 12 bytes: scale
        fwrite(Entity->Transform.Scale.Elements, sizeof(f32), 3, File);

        // 1 byte: has mesh
        fwrite(&Entity->HasMesh, sizeof(bool), 1, File);
        // 256 bytes: mesh path
        fwrite(Entity->MeshPath, sizeof(char), 256, File);

        // 1 byte: has audio
        fwrite(&Entity->HasAudio, sizeof(bool), 1, File);
        // 4 bytes: Source type
        fwrite(&Entity->SourceType, sizeof(u32), 1, File);
        // 1 byte: Looping
        fwrite(&Entity->Source.Looping, sizeof(bool), 1, File);
        // 4 bytes: Volume
        fwrite(&Entity->Source.Volume, sizeof(f32), 1, File);
        // 4 bytes: Pitch
        fwrite(&Entity->Source.Pitch, sizeof(f32), 1, File);
        // 256 bytes: audio path
        fwrite(Entity->AudioPath, sizeof(char), 256, File);
    }

    fclose(File);
}

void DeserialiseScene(scene* Scene, const std::string& Path)
{
    FILE* File = fopen(Path.c_str(), "rb");
    if (!File) {
        LogError("Failed to read scene file: %s", Path.c_str());
        return;
    }

    fread(&Scene->EntityCount, sizeof(u32), 1, File);
    for (i32 EntityIndex = 0; EntityIndex < Scene->EntityCount; EntityIndex++) {
        entity* Entity = &Scene->Entities[EntityIndex];

        // 12 bytes: translation
        fread(Entity->Transform.Translation.Elements, sizeof(f32), 3, File);
        // 16 bytes: rotation
        fread(Entity->Transform.Rotation.Elements, sizeof(f32), 4 , File);
        // 12 bytes: scale
        fread(Entity->Transform.Scale.Elements, sizeof(f32), 3, File);

        // 1 byte: has mesh
        fread(&Entity->HasMesh, sizeof(bool), 1, File);
        // 256 bytes: mesh path
        fread(Entity->MeshPath, sizeof(char), 256, File);

        // 1 byte: has audio
        fread(&Entity->HasAudio, sizeof(bool), 1, File);
        // 4 bytes: Source type
        fread(&Entity->SourceType, sizeof(u32), 1, File);
        // 1 byte: Looping
        fread(&Entity->Source.Looping, sizeof(bool), 1, File);
        // 4 bytes: Volume
        fread(&Entity->Source.Volume, sizeof(f32), 1, File);
        // 4 bytes: Pitch
        fread(&Entity->Source.Pitch, sizeof(f32), 1, File);
        // 256 bytes: audio path
        fread(Entity->AudioPath, sizeof(char), 256, File);

        if (Entity->HasMesh) {
            GpuMeshLoad(&Entity->Mesh, std::string(Entity->MeshPath));
        }
        if (Entity->HasAudio) {
            AudioSourceCreate(&Entity->Source);
            AudioSourceSetLoop(&Entity->Source, Entity->Source.Looping);
            AudioSourceSetVolume(&Entity->Source, Entity->Source.Volume);
            AudioSourceSetPitch(&Entity->Source, Entity->Source.Pitch);
            AudioSourceLoad(&Entity->Source, Entity->AudioPath, Entity->SourceType);
        }
    }

    fclose(File);
}