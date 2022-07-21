#include "backrooms_platform.h"
#include "backrooms_logger.h"

#if defined(BACKROOMS_WINDOWS)

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>

// NOTE(milo): These should be loaded from file.
#define GAME_WINDOW_CLASS_NAME "GameWindowClass"
#define GAME_WINDOW_TITLE "Voyage"
#define GAME_DEFAULT_WIDTH 1280
#define GAME_DEFAULT_HEIGHT 720

typedef DWORD (WINAPI* PFN_XINPUT_GET_STATE)(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD (WINAPI* PFN_XINPUT_SET_STATE)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
typedef DWORD (WINAPI* PFN_XINPUT_GET_BATTERY_INFORMATION)(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation);
typedef HRESULT (WINAPI* PFN_XAUDIO2_CREATE)(IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

PFN_XINPUT_GET_STATE XInputGetStateProc;
PFN_XINPUT_SET_STATE XInputSetStateProc;
PFN_XINPUT_GET_BATTERY_INFORMATION XInputGetBatteryInformationProc;
PFN_XAUDIO2_CREATE XAudio2CreateProc;

struct game_state
{
    HINSTANCE Instance;
    HWND WindowHandle;

    platform_dynamic_lib InputLibrary;
    platform_dynamic_lib AudioLibrary;
};

platform_config PlatformConfiguration;
game_state State;

f32 Normalize(f32 Input, f32 Min, f32 Max);
f32 ApplyDeadzone(f32 Value, f32 MaxValue, f32 Deadzone);

void PlatformMessageBox(const char* Message, bool Error)
{
    MessageBoxA(NULL, Message, Error ? "Error!" : "Message Box", MB_OK | (Error ? MB_ICONERROR : MB_ICONINFORMATION));
}

void PlatformDLLInit(platform_dynamic_lib* Library, const char* Path)
{
    Library->InternalHandle = LoadLibraryA(Path);
    if (!Library->InternalHandle)
        LogCritical("Failed to load dynamic library (%s)", Path);
    Library->Path = Path;    
}

void PlatformDLLExit(platform_dynamic_lib* Library)
{
    if (Library->InternalHandle)
        FreeLibrary((HMODULE)Library->InternalHandle);
}

void* PlatformDLLGet(platform_dynamic_lib* Library, const char* FunctionName)
{
    return GetProcAddress((HMODULE)Library->InternalHandle, FunctionName);
}

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_CREATE: {
            PlatformConfiguration.Running = true;
            break;
        }
        case WM_DESTROY: {
            PlatformConfiguration.Running = false;
            break;
        }
        default: {
            return DefWindowProc(Window, Message, WParam, LParam);
        }
    }

    return 0;
}

void Win32Create(HINSTANCE Instance)
{
    PlatformConfiguration.Width = GAME_DEFAULT_WIDTH;
    PlatformConfiguration.Height = GAME_DEFAULT_HEIGHT;
    State.Instance = Instance;

    CODE_BLOCK("Window Creation")
    {
        WNDCLASSA WindowClass = {};
        WindowClass.lpfnWndProc = WindowProc;
        WindowClass.hInstance = Instance;
        WindowClass.lpszClassName = GAME_WINDOW_CLASS_NAME;
        WindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
        WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassA(&WindowClass);

        State.WindowHandle = CreateWindowA(WindowClass.lpszClassName, 
                                           GAME_WINDOW_TITLE, 
                                           WS_OVERLAPPEDWINDOW, 
                                           CW_USEDEFAULT, 
                                           CW_USEDEFAULT, 
                                           PlatformConfiguration.Width,
                                           PlatformConfiguration.Height, 
                                           NULL, 
                                           NULL, 
                                           Instance,
                                           NULL);
        if (!State.WindowHandle) {
            LogCritical("Failed to create window!");
        }

        ShowWindow(State.WindowHandle, SW_SHOW);
        UpdateWindow(State.WindowHandle);
    }

    CODE_BLOCK("DLL Loading")
    {
        CODE_BLOCK("XInput")
        {
            PlatformDLLInit(&State.InputLibrary, "xinput1_4.dll");

            XInputGetStateProc = (PFN_XINPUT_GET_STATE)PlatformDLLGet(&State.InputLibrary, "XInputGetState");
            if (!XInputGetStateProc) {
                LogCritical("Failed to load XInputGetState function!");
            }

            XInputSetStateProc = (PFN_XINPUT_SET_STATE)PlatformDLLGet(&State.InputLibrary, "XInputSetState");
            if (!XInputGetStateProc) {
                LogCritical("Failed to load XInputSetState function!");
            }

            XInputGetBatteryInformationProc = (PFN_XINPUT_GET_BATTERY_INFORMATION)PlatformDLLGet(&State.InputLibrary, "XInputGetBatteryInformation");
            if (!XInputGetBatteryInformationProc) {
                LogCritical("Failed to load XInputGetBatteryInformation function!");
            }

            LogInfo("Loaded xinput1_4.dll");
        }

        CODE_BLOCK("XAudio2")
        {
            PlatformDLLInit(&State.AudioLibrary, "xaudio2_9.dll");

            XAudio2CreateProc = (PFN_XAUDIO2_CREATE)PlatformDLLGet(&State.AudioLibrary, "XAudio2Create");
            if (!XAudio2CreateProc) {
                LogCritical("Failed to load XAudio2Create function!");
            }

            LogInfo("Loaded xaudio2_9.dll");
        }
    }
}

void XInputUpdate()
{
    for (u16 GamepadIndex = 0; GamepadIndex < GAMEPAD_MAX_PLAYERS; GamepadIndex++)
    {
        XINPUT_STATE ControllerState;
        ZeroMemory(&ControllerState, sizeof(XINPUT_STATE));

        DWORD Result = XInputGetStateProc(GamepadIndex, &ControllerState);
        bool Connected = (Result == ERROR_SUCCESS);

        if (Connected)
        {
            CODE_BLOCK("Buttons")
            {
                GamepadProcessButtonState(GamepadIndex, GamepadButton_A, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_B, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_X, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_Y, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_Dpad_Up, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_Dpad_Down, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_Dpad_Left, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_Dpad_Right, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_LeftThumb, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_LeftShoulder, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_RightThumb, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0);
                GamepadProcessButtonState(GamepadIndex, GamepadButton_RightShoulder, (ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
            }

            CODE_BLOCK("Analog triggers")
            {
                f32 LeftTrigger = (f32)ControllerState.Gamepad.bLeftTrigger / 255;
                f32 RightTrigger = (f32)ControllerState.Gamepad.bRightTrigger / 255;

                GamepadProcessTrigger(GamepadIndex, GamepadPhysicalLocation_Left, LeftTrigger);
                GamepadProcessTrigger(GamepadIndex, GamepadPhysicalLocation_Right, RightTrigger);
            }
        
            CODE_BLOCK("Joysticks")
            {
                f32 NormLX = Normalize((f32)ControllerState.Gamepad.sThumbLX, -32767, 32767);
                f32 NormLY = Normalize((f32)ControllerState.Gamepad.sThumbLY, -32767, 32767);
                f32 NormRX = Normalize((f32)ControllerState.Gamepad.sThumbRX, -32767, 32767);
                f32 NormRY = Normalize((f32)ControllerState.Gamepad.sThumbRY, -32767, 32767);
                
                f32 LX = 0.0f;
                f32 LY = 0.0f;
                f32 RX = 0.0f;
                f32 RY = 0.0f;

                if constexpr(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE <= 1.0f || XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE <= 1.0f)
                {
                    LX = ApplyDeadzone(NormLX,  1.0f, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    LY = ApplyDeadzone(NormLY,  1.0f, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
                    RX = ApplyDeadzone(NormRX,  1.0f, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    RY = ApplyDeadzone(NormRY,  1.0f, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
                }
                else
                {
                    LX = ApplyDeadzone(NormLX,  1.0f, Normalize(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, SHRT_MIN, SHRT_MAX));
                    LY = ApplyDeadzone(NormLY,  1.0f, Normalize(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, SHRT_MIN, SHRT_MAX));
                    RX = ApplyDeadzone(NormRX,  1.0f, Normalize(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, SHRT_MIN, SHRT_MAX));
                    RY = ApplyDeadzone(NormRY,  1.0f, Normalize(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, SHRT_MIN, SHRT_MAX));
                }

                GamepadProcessJoystick(GamepadIndex, GamepadPhysicalLocation_Left, LX, LY);
                GamepadProcessJoystick(GamepadIndex, GamepadPhysicalLocation_Right, RX, RY);
            }

            CODE_BLOCK("Vibration")
            {
                XINPUT_VIBRATION Vibration;
                f32 RightSpeed, LeftSpeed;
                GamepadGetVibrationValue(GamepadIndex, &RightSpeed, &LeftSpeed);

                u16 Right = RightSpeed * 65335.0f;
                u16 Left = LeftSpeed * 65335.0f;
                Vibration.wLeftMotorSpeed = Left;
                Vibration.wRightMotorSpeed = Right;

                XInputSetStateProc(GamepadIndex, &Vibration);
            }
        }
    }
}

void Win32Update()
{
    MSG Message;
    while (PeekMessageA(&Message, State.WindowHandle, 0, 0, PM_REMOVE)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
}

void Win32Destroy()
{
    PlatformDLLExit(&State.AudioLibrary);
    PlatformDLLExit(&State.InputLibrary);

    DestroyWindow(State.WindowHandle);
}

int main()
{
    Win32Create(GetModuleHandle(NULL));
    while (PlatformConfiguration.Running) {
        Win32Update();
        XInputUpdate();
    }
    Win32Destroy();
}

f32 Normalize(f32 Input, f32 Min, f32 Max)
{
    f32 Average = (Min + Max) / 2;
    f32 Range = (Max - Min) / 2;
    return (Input - Average) / Range;
}

f32 ApplyDeadzone(f32 Value, f32 MaxValue, f32 Deadzone)
{
    if (Value < -Deadzone)
        Value += Deadzone;
    else if (Value > Deadzone)
        Value -= Deadzone;
    else
        return 0;
    f32 NormValue = Value / (MaxValue - Deadzone);
    return max(-1.0f, min(NormValue, 1.0f));
}

#endif