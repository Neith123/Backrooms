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

enum log_color
{
    LogColor_CyanInfo,
    LogColor_YellowWarn,
    LogColor_ErrorCriticalRed
};

extern platform_config PlatformConfiguration;

// NOTE(milo): Debug tools
void PlatformMessageBox(const char* Message, bool Error);
void PlatformSetLogColor(log_color Color);

// NOTE(milo): DLL
void PlatformDLLInit(platform_dynamic_lib* Library, const char* Path);
void PlatformDLLExit(platform_dynamic_lib* Library);
void* PlatformDLLGet(platform_dynamic_lib* Library, const char* FunctionName);