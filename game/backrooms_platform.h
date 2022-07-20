#pragma once

#include "backrooms_common.h"

struct platform_config
{
    bool Running;
    bool VerticalSync;
    i32 Width;
    i32 Height;
};

struct platform_dynamic_lib
{
    void* InternalHandle;
    const char* Path;
};

extern platform_config PlatformConfiguration;

// NOTE(milo): DLL
void PlatformDLLInit(platform_dynamic_lib* Library, const char* Path);
void PlatformDLLExit(platform_dynamic_lib* Library);
void* PlatformDLLGet(platform_dynamic_lib* Library, const char* FunctionName);