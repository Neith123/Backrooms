#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>

#include "backrooms_platform.h"
#include "backrooms_logger.h"

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
    }
    Win32Destroy();
}