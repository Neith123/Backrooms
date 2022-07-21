#pragma once

#include "backrooms_common.h"

#define GAMEPAD_MAX_PLAYERS 4

enum keyboard_keys 
{
    KeyboardKey_Backspace = 0x08,
    KeyboardKey_Enter = 0x0D,
    KeyboardKey_Tab = 0x09,
    KeyboardKey_Shift = 0x10,
    KeyboardKey_Control = 0x11,
    KeyboardKey_Pause = 0x13,
    KeyboardKey_Capital = 0x14,
    KeyboardKey_Escape = 0x1B,
    KeyboardKey_Convert = 0x1C,
    KeyboardKey_NonConvert = 0x1D,
    KeyboardKey_Accept = 0x1E,
    KeyboardKey_ModeChange = 0x1F,
    KeyboardKey_Space = 0x20,
    KeyboardKey_Prior = 0x21,
    KeyboardKey_Next = 0x22,
    KeyboardKey_End = 0x23,
    KeyboardKey_Home = 0x24,
    KeyboardKey_Left = 0x25,
    KeyboardKey_Up = 0x26,
    KeyboardKey_Right = 0x27,
    KeyboardKey_Down = 0x28,
    KeyboardKey_Select = 0x29,
    KeyboardKey_Print = 0x2A,
    KeyboardKey_Execute = 0x2B,
    KeyboardKey_Snapshot = 0x2C,
    KeyboardKey_Insert = 0x2D,
    KeyboardKey_Delete = 0x2E,
    KeyboardKey_Help = 0x2F,
    KeyboardKey_0 = 0x30,
    KeyboardKey_1 = 0x31,
    KeyboardKey_2 = 0x32,
    KeyboardKey_3 = 0x33,
    KeyboardKey_4 = 0x34,
    KeyboardKey_5 = 0x35,
    KeyboardKey_6 = 0x36,
    KeyboardKey_7 = 0x37,
    KeyboardKey_8 = 0x38,
    KeyboardKey_9 = 0x39,
    KeyboardKey_A = 0x41,
    KeyboardKey_B = 0x42,
    KeyboardKey_C = 0x43,
    KeyboardKey_D = 0x44,
    KeyboardKey_E = 0x45,
    KeyboardKey_F = 0x46,
    KeyboardKey_G = 0x47,
    KeyboardKey_H = 0x48,
    KeyboardKey_I = 0x49,
    KeyboardKey_J = 0x4A,
    KeyboardKey_K = 0x4B,
    KeyboardKey_L = 0x4C,
    KeyboardKey_M = 0x4D,
    KeyboardKey_N = 0x4E,
    KeyboardKey_O = 0x4F,
    KeyboardKey_P = 0x50,
    KeyboardKey_Q = 0x51,
    KeyboardKey_R = 0x52,
    KeyboardKey_S = 0x53,
    KeyboardKey_T = 0x54,
    KeyboardKey_U = 0x55,
    KeyboardKey_V = 0x56,
    KeyboardKey_W = 0x57,
    KeyboardKey_X = 0x58,
    KeyboardKey_Y = 0x59,
    KeyboardKey_Z = 0x5A,
    KeyboardKey_LeftWin = 0x5B,
    KeyboardKey_RightWin = 0x5C,
    KeyboardKey_Apps = 0x5D,
    KeyboardKey_Sleep = 0x5F,
    KeyboardKey_NumPad0 = 0x60,
    KeyboardKey_NumPad1 = 0x61,
    KeyboardKey_NumPad2 = 0x62,
    KeyboardKey_NumPad3 = 0x63,
    KeyboardKey_NumPad4 = 0x64,
    KeyboardKey_NumPad5 = 0x65,
    KeyboardKey_NumPad6 = 0x66,
    KeyboardKey_NumPad7 = 0x67,
    KeyboardKey_NumPad8 = 0x68,
    KeyboardKey_NumPad9 = 0x69,
    KeyboardKey_Multiply = 0x6A,
    KeyboardKey_Add = 0x6B,
    KeyboardKey_Separator = 0x6C,
    KeyboardKey_Substract = 0x6D,
    KeyboardKey_Decimal = 0x6E,
    KeyboardKey_Divide = 0x6F,
    KeyboardKey_F1 = 0x70,
    KeyboardKey_F2 = 0x71,
    KeyboardKey_F3 = 0x72,
    KeyboardKey_F4 = 0x73,
    KeyboardKey_F5 = 0x74,
    KeyboardKey_F6 = 0x75,
    KeyboardKey_F7 = 0x76,
    KeyboardKey_F8 = 0x77,
    KeyboardKey_F9 = 0x78,
    KeyboardKey_F10 = 0x79,
    KeyboardKey_F11 = 0x7A,
    KeyboardKey_F12 = 0x7B,
    KeyboardKey_F13 = 0x7C,
    KeyboardKey_F14 = 0x7D,
    KeyboardKey_F15 = 0x7E,
    KeyboardKey_F16 = 0x7F,
    KeyboardKey_F17 = 0x80,
    KeyboardKey_F18 = 0x81,
    KeyboardKey_F19 = 0x82,
    KeyboardKey_F20 = 0x83,
    KeyboardKey_F21 = 0x84,
    KeyboardKey_F22 = 0x85,
    KeyboardKey_F23 = 0x86,
    KeyboardKey_F24 = 0x87,
    KeyboardKey_NumLock = 0x90,
    KeyboardKey_Scroll = 0x91,
    KeyboardKey_NumPad_Equal = 0x92,
    KeyboardKey_LeftShift = 0xA0,
    KeyboardKey_RightShift = 0xA1,
    KeyboardKey_LeftControl = 0xA2,
    KeyboardKey_RightControl = 0xA3,
    KeyboardKey_LeftAlt = 0xA4,
    KeyboardKey_RightAlt = 0xA5,
    KeyboardKey_SemiColon = 0xBA,
    KeyboardKey_Plus = 0xBB,
    KeyboardKey_Comma = 0xBC,
    KeyboardKey_Minus = 0xBD,
    KeyboardKey_Period = 0xBE,
    KeyboardKey_Slash = 0xBF,
    KeyboardKey_Grave = 0xC0,
    KeyboardKeys_MaxKeys
};

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

bool KeyboardIsKeyDown(keyboard_keys Key);
bool KeyboardIsKeyUp(keyboard_keys Key);
void KeyboardProcessKey(keyboard_keys Key, bool State);