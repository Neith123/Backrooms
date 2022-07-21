#include "backrooms_platform.h"

struct gamepad_state
{
    bool Buttons[GamepadButton_MaxButtons];
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

void GamepadProcessButtonState(i32 GamepadIndex, gamepad_buttons Button, bool State)
{
    InputState.GamepadState[GamepadIndex].Buttons[Button] = State;
}