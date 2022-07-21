#pragma once

#include "backrooms_common.h"

#define GAMEPAD_MAX_PLAYERS 4

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

enum gamepad_buttons
{
    GamepadButton_Dpad_Up,
    GamepadButton_Dpad_Down,
    GamepadButton_Dpad_Left,
    GamepadButton_Dpad_Right,
    GamepadButton_Start,
    GamepadButton_Back,
    GamepadButton_LeftThumb,
    GamepadButton_RightThumb,
    GamepadButton_LeftShoulder,
    GamepadButton_RightShoulder,
    GamepadButton_A,
    GamepadButton_B,
    GamepadButton_Y,
    GamepadButton_X,
    GamepadButton_MaxButtons
};

enum gamepad_physical_location
{
    GamepadPhysicalLocation_Left,
    GamepadPhysicalLocation_Right,
    GamepadPhysicalLocation_MaxLocations
};

extern platform_config PlatformConfiguration;

// NOTE(milo): Debug tools
void PlatformMessageBox(const char* Message, bool Error);

// NOTE(milo): Input functions
bool GamepadIsButtonPressed(i32 GamepadIndex, gamepad_buttons Button);
bool GamepadIsButtonReleased(i32 GamepadIndex, gamepad_buttons Button);
f32 GamepadGetTriggerValue(i32 GamepadIndex, gamepad_physical_location Location);

void GamepadProcessButtonState(i32 GamepadIndex, gamepad_buttons Button, bool State);
void GamepadProcessTrigger(i32 GamepadIndex, gamepad_physical_location Location, f32 Value);

// NOTE(milo): DLL
void PlatformDLLInit(platform_dynamic_lib* Library, const char* Path);
void PlatformDLLExit(platform_dynamic_lib* Library);
void* PlatformDLLGet(platform_dynamic_lib* Library, const char* FunctionName);