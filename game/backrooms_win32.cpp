#include "backrooms_platform.h"
#include "backrooms_logger.h"
#include "backrooms_input.h"
#include "backrooms_audio.h"
#include "backrooms_rhi.h"
#include "backrooms.h"

#if defined(BACKROOMS_WINDOWS)

#include <Windows.h>
#include <windowsx.h>
#include <Xinput.h>
#include <xaudio2.h>

#include <fstream>
#include <sstream>

// NOTE(milo): These should be loaded from file.
#define GAME_WINDOW_CLASS_NAME "GameWindowClass"
#define GAME_WINDOW_TITLE "Backrooms"
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

struct audio_state
{
    IXAudio2* Device;
    IXAudio2MasteringVoice* MasteringVoice;
};

platform_config PlatformConfiguration;
game_state State;
audio_state AudioState;

f32 Normalize(f32 Input, f32 Min, f32 Max);
f32 ApplyDeadzone(f32 Value, f32 MaxValue, f32 Deadzone);

void PlatformMessageBox(const char* Message, bool Error)
{
    MessageBoxA(NULL, Message, Error ? "Error!" : "Message Box", MB_OK | (Error ? MB_ICONERROR : MB_ICONINFORMATION));
}

void PlatformSetLogColor(log_color Color)
{
    static u8 Levels[3] = {11, 6, 4};
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Levels[Color]);
}

std::string PlatformReadFile(const char* Path)
{
    std::ifstream Stream(Path);
    if (!Stream.is_open()) {
        LogError("Failed to open file: %s", Path);
        return "";
    }
    std::stringstream StringStream;
    StringStream << Stream.rdbuf();
    Stream.close();
    return StringStream.str();
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
        case WM_SIZE: {
            if (VideoReady()) {
                VideoResize((u32)LOWORD(LParam), (u32)HIWORD(LParam));
            }
            break;
        }
        case WM_MOUSEMOVE: {
            // Mouse move
            i32 XPosition = GET_X_LPARAM(LParam);
            i32 YPosition = GET_Y_LPARAM(WParam);
			
            MouseProcessPosition(XPosition, YPosition);
        } break;
        case WM_MOUSEWHEEL: {
            i32 ZDelta = GET_WHEEL_DELTA_WPARAM(WParam);
            if (ZDelta != 0) {
                ZDelta = (ZDelta < 0) ? -1 : 1;
                MouseProcessWheel(ZDelta);
            }
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            bool Pressed = (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN);
            keyboard_keys Key = (keyboard_keys)WParam;
		
            bool IsExtended = (HIWORD(LParam) & KF_EXTENDED) == KF_EXTENDED;
			
            if (WParam == VK_MENU) {
                Key = IsExtended ? KeyboardKey_RightAlt : KeyboardKey_LeftAlt;
            } else if (WParam == VK_SHIFT) {
                u32 LeftShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                u32 Scancode = ((LParam & (0xFF << 16)) >> 16);
                Key = Scancode == LeftShift ? KeyboardKey_LeftShift : KeyboardKey_RightShift;
            } else if (WParam == VK_CONTROL) {
                Key = IsExtended ? KeyboardKey_RightControl : KeyboardKey_LeftControl;
            }
			
            KeyboardProcessKey(Key, Pressed);
			
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            bool Pressed = Message == WM_LBUTTONDOWN || Message == WM_RBUTTONDOWN || Message == WM_MBUTTONDOWN;
            mouse_buttons Button = MouseButton_MaxButtons;
            switch (Message) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
				Button = MouseButton_Left;
				break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
				Button = MouseButton_Middle;
				break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
				Button = MouseButton_Right;
				break;
            }
			
            if (Button != MouseButton_MaxButtons) {
                MouseProcessButton(Button, Pressed);
            }
        } break;
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

            LogInfo("Loaded xinput1_4.dll.");
        }

        CODE_BLOCK("XAudio2")
        {
            PlatformDLLInit(&State.AudioLibrary, "xaudio2_9.dll");

            XAudio2CreateProc = (PFN_XAUDIO2_CREATE)PlatformDLLGet(&State.AudioLibrary, "XAudio2Create");
            if (!XAudio2CreateProc) {
                LogCritical("Failed to load XAudio2Create function!");
            }

            LogInfo("Loaded xaudio2_9.dll.");
        }
    }

    AudioInit();
    VideoInit((void*)State.WindowHandle);
    GameInit();
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
    GameExit();
    VideoExit();
    AudioExit();

    PlatformDLLExit(&State.AudioLibrary);
    PlatformDLLExit(&State.InputLibrary);

    DestroyWindow(State.WindowHandle);
}

void AudioInit()
{
    HRESULT Result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(Result)) {
        LogCritical("Failed to initialize COM.");
    }

    u32 Flags = 0;
#if defined(_DEBUG)
    Flags |= XAUDIO2_DEBUG_ENGINE;
#endif

    Result = XAudio2CreateProc(&AudioState.Device, Flags, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(Result)) {
        LogCritical("Failed to initialise XAudio2!");
    }

#if defined(_DEBUG)
    XAUDIO2_DEBUG_CONFIGURATION DebugConfig = {};
	DebugConfig.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
	DebugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
    AudioState.Device->SetDebugConfiguration(&DebugConfig);
#endif

    Result = AudioState.Device->CreateMasteringVoice(&AudioState.MasteringVoice, DEFAULT_AUDIO_CHANNELS, DEFAULT_AUDIO_SAMPLE_RATE, 0, NULL, NULL, AudioCategory_GameMedia);
    if (FAILED(Result)) {
        LogCritical("Failed to create XAudio2 mastering voice!");
    }

    Result = AudioState.Device->StartEngine();
    if (FAILED(Result)) {
        LogCritical("Failed to start XAudio2 engine!");
    }

    LogInfo("Initialised XAudio2.");
}

void AudioExit()
{
    AudioState.MasteringVoice->DestroyVoice();
    AudioState.Device->StopEngine();
    AudioState.Device->Release();
}

void AudioSourceCreate(audio_source* Source)
{
    Source->Looping = false;
    Source->Volume = 1.0f;
    Source->Pitch = 1.0f;
    Source->Samples = nullptr;
    Source->BackendData = nullptr;

    WAVEFORMATEX WaveFormat = {};
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM; // PCM audio format
	WaveFormat.wBitsPerSample = 16; // i16
	WaveFormat.nChannels = DEFAULT_AUDIO_CHANNELS; // 2 channels commonly
	WaveFormat.nSamplesPerSec = DEFAULT_AUDIO_SAMPLE_RATE; // 48khz or 48000 sample rate commonly
	WaveFormat.nAvgBytesPerSec = (WaveFormat.wBitsPerSample * WaveFormat.nSamplesPerSec * WaveFormat.nChannels) / 8;
	WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8; // 4 bytes commonly
	WaveFormat.cbSize = 0;

    HRESULT Result = AudioState.Device->CreateSourceVoice((IXAudio2SourceVoice**)&Source->BackendData, &WaveFormat, XAUDIO2_VOICE_USEFILTER, 1024.0f, NULL, NULL, NULL);
    if (FAILED(Result)) {
        LogCritical("Failed to create source voice!");
    }
}

void AudioSourceLoad(audio_source* Source, const char* Path, audio_source_type Type)
{
    IXAudio2SourceVoice* SourceVoice = (IXAudio2SourceVoice*)Source->BackendData;

    Source->Type = Type;

    u64 TotalPCMFrameCount = 0;

    switch (Type) {
        case AudioSourceType_WAV: {
            if (!drwav_init_file(&Source->Loaders.Wave, Path, NULL)) {
                LogError("Failed to load wave file: %s", Path);
                return;
            }

            TotalPCMFrameCount = Source->Loaders.Wave.totalPCMFrameCount;
            Source->Samples = (i16*)malloc(TotalPCMFrameCount * DEFAULT_AUDIO_CHANNELS * sizeof(i16));
            drwav_read_pcm_frames_s16(&Source->Loaders.Wave, TotalPCMFrameCount, Source->Samples);

            break;
        }
        case AudioSourceType_MP3: {
            if (!drmp3_init_file(&Source->Loaders.MP3, Path, NULL)) {
                LogError("Failed to load mp3 file: %s", Path);
                return;
            }

            TotalPCMFrameCount = drmp3_get_pcm_frame_count(&Source->Loaders.MP3);
            Source->Samples = (i16*)malloc(TotalPCMFrameCount * DEFAULT_AUDIO_CHANNELS * sizeof(i16));
            drmp3_read_pcm_frames_s16(&Source->Loaders.MP3, TotalPCMFrameCount, Source->Samples);

            break;
        }
        case AudioSourceType_FLAC: {
            Source->Loaders.Flac = drflac_open_file(Path, NULL);
            if (!Source->Loaders.Flac) {
                LogCritical("Failed to load flac file: %s", Path);
            }

            TotalPCMFrameCount = Source->Loaders.Flac->totalPCMFrameCount;
            Source->Samples = (i16*)malloc(TotalPCMFrameCount * DEFAULT_AUDIO_CHANNELS * sizeof(i16));
            drflac_read_pcm_frames_s16(Source->Loaders.Flac, TotalPCMFrameCount, Source->Samples);

            break;
        }
    }

    XAUDIO2_BUFFER AudioBuffer = {};
	AudioBuffer.Flags = 0;
	AudioBuffer.AudioBytes = TotalPCMFrameCount * DEFAULT_AUDIO_CHANNELS * sizeof(i16);
	AudioBuffer.pAudioData = (BYTE*)Source->Samples;
	AudioBuffer.PlayBegin = 0;
	AudioBuffer.PlayLength = 0; // Play the entire buffer
	AudioBuffer.LoopBegin = 0;
	AudioBuffer.LoopLength = 0;
	AudioBuffer.LoopCount = Source->Looping ? XAUDIO2_LOOP_INFINITE : 0;
	AudioBuffer.pContext = NULL;

    HRESULT Result = SourceVoice->SubmitSourceBuffer(&AudioBuffer, NULL);
    if (FAILED(Result)) {
        LogError("Failed to submit XAudio2 source buffer!");
        return;
    }

    LogInfo("Loaded audio source: %s", Path);
}

void AudioSourcePlay(audio_source* Source)
{
    IXAudio2SourceVoice* SourceVoice = (IXAudio2SourceVoice*)Source->BackendData;

    if (FAILED(SourceVoice->Start(0, XAUDIO2_COMMIT_NOW))) {
        LogError("Failed to start source voice!");
        return;
    }
}

void AudioSourceStop(audio_source* Source)
{
    IXAudio2SourceVoice* SourceVoice = (IXAudio2SourceVoice*)Source->BackendData;
    SourceVoice->Stop(0, XAUDIO2_COMMIT_NOW);
}

void AudioSourceSetVolume(audio_source* Source, f32 Volume)
{
    IXAudio2SourceVoice* SourceVoice = (IXAudio2SourceVoice*)Source->BackendData;

    Source->Volume = Volume;
    SourceVoice->SetVolume(Volume, XAUDIO2_COMMIT_NOW);
}

void AudioSourceSetPitch(audio_source* Source, f32 Pitch)
{
    IXAudio2SourceVoice* SourceVoice = (IXAudio2SourceVoice*)Source->BackendData;

    Source->Pitch = Pitch;
    SourceVoice->SetFrequencyRatio(Pitch, XAUDIO2_COMMIT_NOW);
}

void AudioSourceSetLoop(audio_source* Source, bool Loop)
{
    Source->Looping = Loop;
}

void AudioSourceDestroy(audio_source* Source)
{
    if (Source->Samples) {
        free(Source->Samples);
        switch (Source->Type) {
            case AudioSourceType_FLAC: {
                drflac_close(Source->Loaders.Flac);
                break;
            }
            case AudioSourceType_MP3: {
                drmp3_uninit(&Source->Loaders.MP3);
                break;
            }
            case AudioSourceType_WAV: {
                drwav_uninit(&Source->Loaders.Wave);
                break;
            }
        }
    }

    if (Source->BackendData) {
        IXAudio2SourceVoice* SourceVoice = (IXAudio2SourceVoice*)Source->BackendData;
        SourceVoice->DestroyVoice();
    }
}

int main()
{
    Win32Create(GetModuleHandle(NULL));
    while (PlatformConfiguration.Running) {
        Win32Update();
        XInputUpdate();

        GameUpdate();

        VideoPresent();
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