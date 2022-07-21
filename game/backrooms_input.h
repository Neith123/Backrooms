#pragma once

#include "backrooms_common.h"

#define GAMEPAD_MAX_PLAYERS 4

enum mouse_buttons
{
    MouseButton_Left,
    MouseButton_Right,
    MouseButton_Middle,
    MouseButton_MaxButtons
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

bool GamepadIsButtonPressed(i32 GamepadIndex, gamepad_buttons Button);
bool GamepadIsButtonReleased(i32 GamepadIndex, gamepad_buttons Button);
f32 GamepadGetTriggerValue(i32 GamepadIndex, gamepad_physical_location Location);
void GamepadGetJoystickValue(i32 GamepadIndex, gamepad_physical_location Location, f32* X, f32* Y);
void GamepadGetVibrationValue(i32 GamepadIndex, f32* Left, f32* Right);

void GamepadProcessButtonState(i32 GamepadIndex, gamepad_buttons Button, bool State);
void GamepadProcessTrigger(i32 GamepadIndex, gamepad_physical_location Location, f32 Value);
void GamepadProcessJoystick(i32 GamepadIndex, gamepad_physical_location Location, f32 X, f32 Y);
void GamepadSetVibrationValue(i32 GamepadIndex, f32 Left, f32 Right);
void GamepadResetVibration(i32 GamepadIndex);

void MouseGetPosition(i32* X, i32* Y);
i8 MouseGetWheel();
bool MouseIsButtonPressed(mouse_buttons Button);
bool MouseIsButtonReleased(mouse_buttons Button);

void MouseProcessPosition(i32 X, i32 Y);
void MouseProcessWheel(i8 Wheel);
void MouseProcessButton(mouse_buttons Button, bool State);