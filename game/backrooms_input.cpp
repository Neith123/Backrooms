#include "backrooms_platform.h"

struct gamepad_state
{
    bool Buttons[GamepadButton_MaxButtons];

    f32 Triggers[GamepadPhysicalLocation_MaxLocations];
    f32 Joysticks[GamepadPhysicalLocation_MaxLocations][2];
    f32 Vibration[2];
};

struct input_state
{
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