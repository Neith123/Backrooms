#include "backrooms_input.h"

struct keyboard_state
{
    bool Keys[KeyboardKeys_MaxKeys];
};

struct gamepad_state
{
    bool Buttons[GamepadButton_MaxButtons];

    f32 Triggers[GamepadPhysicalLocation_MaxLocations];
    f32 Joysticks[GamepadPhysicalLocation_MaxLocations][2];
    f32 Vibration[2];
};

struct mouse_state
{
    bool Buttons[MouseButton_MaxButtons];

    i8 Wheel;
    i16 Position[2];
};

struct input_state
{
    keyboard_state KeyboardState;
    mouse_state MouseState;
    gamepad_state GamepadState[GAMEPAD_MAX_PLAYERS];
};

static input_state InputState;

bool GamepadIsButtonPressed(i32 GamepadIndex, gamepad_buttons Button)
{
    return InputState.GamepadState[GamepadIndex].Buttons[Button] == true;
}

bool GamepadIsButtonReleased(i32 GamepadIndex, gamepad_buttons Button)
{
    return InputState.GamepadState[GamepadIndex].Buttons[Button] == false;
}

f32 GamepadGetTriggerValue(i32 GamepadIndex, gamepad_physical_location Location)
{
    return InputState.GamepadState[GamepadIndex].Triggers[Location];
}

void GamepadGetJoystickValue(i32 GamepadIndex, gamepad_physical_location Location, f32* X, f32* Y)
{
    *X = InputState.GamepadState[GamepadIndex].Joysticks[Location][0];
    *Y = InputState.GamepadState[GamepadIndex].Joysticks[Location][1];
}

void GamepadGetVibrationValue(i32 GamepadIndex, f32* Left, f32* Right)
{
    *Left = InputState.GamepadState[GamepadIndex].Vibration[0];
    *Right = InputState.GamepadState[GamepadIndex].Vibration[1];
}

void GamepadProcessButtonState(i32 GamepadIndex, gamepad_buttons Button, bool State)
{
    InputState.GamepadState[GamepadIndex].Buttons[Button] = State;
}

void GamepadProcessTrigger(i32 GamepadIndex, gamepad_physical_location Location, f32 Value)
{
    InputState.GamepadState[GamepadIndex].Triggers[Location] = Value;
}

void GamepadProcessJoystick(i32 GamepadIndex, gamepad_physical_location Location, f32 X, f32 Y)
{
    InputState.GamepadState[GamepadIndex].Joysticks[Location][0] = X;
    InputState.GamepadState[GamepadIndex].Joysticks[Location][1] = Y;
}

void GamepadSetVibrationValue(i32 GamepadIndex, f32 Left, f32 Right)
{
    InputState.GamepadState[GamepadIndex].Vibration[0] = Left;
    InputState.GamepadState[GamepadIndex].Vibration[1] = Right;
}

void GamepadResetVibration(i32 GamepadIndex)
{
    GamepadSetVibrationValue(GamepadIndex, 0.0f, 0.0f);
}

void MouseGetPosition(i32* X, i32* Y)
{
    *X = (i32)InputState.MouseState.Position[0];
    *Y = (i32)InputState.MouseState.Position[1];
}

i8 MouseGetWheel()
{
    return InputState.MouseState.Wheel;
}

bool MouseIsButtonPressed(mouse_buttons Button)
{
    return InputState.MouseState.Buttons[Button] == true;
}

bool MouseIsButtonReleased(mouse_buttons Button)
{
    return InputState.MouseState.Buttons[Button] == false;
}

void MouseProcessPosition(i32 X, i32 Y)
{
    InputState.MouseState.Position[0] = X;
    InputState.MouseState.Position[1] = Y;
}

void MouseProcessWheel(i8 Wheel)
{
    InputState.MouseState.Wheel = Wheel;
}

void MouseProcessButton(mouse_buttons Button, bool State)
{
    InputState.MouseState.Buttons[Button] = State;
}

bool KeyboardIsKeyDown(keyboard_keys Key)
{
    return InputState.KeyboardState.Keys[Key] == true;
}

bool KeyboardIsKeyUp(keyboard_keys Key)
{
    return InputState.KeyboardState.Keys[Key] == false;
}

void KeyboardProcessKey(keyboard_keys Key, bool State)
{
    InputState.KeyboardState.Keys[Key] = State;
}