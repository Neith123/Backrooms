#pragma once

#include "backrooms_common.h"

#include <string>

typedef u32 (*PFN_ThreadStart)(void*);

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

struct platform_thread
{
    void* Internal;
    u64 ThreadID;
};

struct platform_mutex
{
    void* Internal;
};

enum log_color
{
    LogColor_CyanInfo,
    LogColor_YellowWarn,
    LogColor_ErrorCriticalRed
};

extern platform_config PlatformConfiguration;

//~ NOTE(milo): Debug tools
void PlatformMessageBox(const char* Message, bool Error);
void PlatformSetLogColor(log_color Color);

//~ NOTE(milo): File IO
std::string PlatformReadFile(const char* Path);

//~ NOTE(milo): DLL
void PlatformDLLInit(platform_dynamic_lib* Library, const char* Path);
void PlatformDLLExit(platform_dynamic_lib* Library);
void* PlatformDLLGet(platform_dynamic_lib* Library, const char* FunctionName);

//~ NOTE(milo): Timer
void PlatformTimerInit();
f32 PlatformTimerGet();

//~ NOTE(milo): Thread
i32 PlatformGetProcessorCount();
u64 PlatformGetThreadID();
void PlatformThreadCreate(PFN_ThreadStart StartFunction, void* Params, bool AutoDetach, platform_thread* Thread);
void PlatformThreadDestroy(platform_thread* Thread);
void PlatformThreadDetach(platform_thread* Thread);
void PlatformThreadCancel(platform_thread* Thread);
bool PlatformThreadActive(platform_thread* Thread);
void PlatformThreadSleep(platform_thread* Thread, u64 Miliseconds);

//~ NOTE(milo): Mutex
void PlatformMutexCreate(platform_mutex* Mutex);
void PlatformMutexDestroy(platform_mutex* Mutex);
bool PlatformMutexLock(platform_mutex* Mutex);
bool PlatformMutexUnlock(platform_mutex* Mutex);