#include <Windows.h>

#include "backrooms_platform.h"
#include "backrooms_logger.h"

// NOTE(milo): These should be loaded from file.
#define GAME_WINDOW_CLASS_NAME "GameWindowClass"
#define GAME_WINDOW_TITLE "Voyage"
#define GAME_DEFAULT_WIDTH 1280
#define GAME_DEFAULT_HEIGHT 720

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
        PlatformDLLInit(&State.InputLibrary, "xinput1_4.dll");
        PlatformDLLInit(&State.AudioLibrary, "xaudio2_9.dll");
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