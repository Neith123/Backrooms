#include "backrooms_camera.h"
#include "backrooms_input.h"

#include <math.h>
#include <string.h>

hmm_vec4 PlaneToVec4(hmm_vec3 Norm, hmm_vec3 Point)
{
    hmm_vec4 Temp;
    Temp.XYZ = Norm;
    Temp.W = HMM_DotVec3(Norm, Point);
    return Temp;
}

void NoClipCameraUpdateVectors(noclip_camera* Camera)
{
    hmm_vec3 Front;
    Front.X = cos(HMM_ToRadians(Camera->Yaw)) * cos(HMM_ToRadians(Camera->Pitch));
    Front.Y = sin(HMM_ToRadians(Camera->Pitch));
    Front.Z = sin(HMM_ToRadians(Camera->Yaw)) * cos(HMM_ToRadians(Camera->Pitch));
    Camera->Front = HMM_NormalizeVec3(Front);

    Camera->Right = HMM_NormalizeVec3(HMM_Cross(Camera->Front, Camera->WorldUp));
    Camera->Up = HMM_NormalizeVec3(HMM_Cross(Camera->Right, Camera->Front));
}

void NoClipCameraInit(noclip_camera* Camera)
{
    memset(Camera, 0, sizeof(noclip_camera));

    Camera->Up.Y = -1.0f;
    Camera->Front.Z = -1.0f;
    Camera->WorldUp.Y = 1.0f;
    Camera->Position.Z = 1.0f;
    Camera->Yaw = CAMERA_DEFAULT_YAW;
    Camera->Pitch = CAMERA_DEFAULT_PITCH;
    Camera->Friction = 10.0f;
    Camera->Acceleration = 20.0f;
    Camera->MaxVelocity = 15.0f;
    Camera->Width = (f32)1280;
    Camera->Height = (f32)720;

    NoClipCameraUpdateVectors(Camera);
}

void NoClipCameraUpdate(noclip_camera* Camera, f32 Delta)
{
    i32 X, Y = 0;
    MouseGetPosition(&X, &Y);

    Camera->MousePos.X = (f32)X;
    Camera->MousePos.Y = (f32)Y;

    Camera->View = HMM_LookAt(Camera->Position, HMM_AddVec3(Camera->Position, Camera->Front), Camera->WorldUp);
    Camera->Projection = HMM_Perspective(75.0f, Camera->Width / Camera->Height, 0.001f, 10000.0f);
}

void NoClipCameraInput(noclip_camera* Camera, f32 Delta)
{
    f32 LX, LY = 0.0f;
    f32 RX, RY = 0.0f;
    GamepadGetJoystickValue(0, GamepadPhysicalLocation_Left, &LX, &LY);
    GamepadGetJoystickValue(0, GamepadPhysicalLocation_Right, &RX, &RY);

    f32 XMultiplier, YMultiplier = 1.0f;
    if (LX > 0.0f) {
        XMultiplier = LX;
    }
    if (LX < 0.0f) {
        XMultiplier = -LX;
    }
    if (LY > 0.0f) {
        YMultiplier = LY;
    }
    if (LY < 0.0f) {
        YMultiplier = -LY;
    }

    f32 SpeedMultiplier = Camera->Acceleration * Delta;
    if (KeyboardIsKeyDown(KeyboardKey_Z))
        Camera->Velocity = HMM_AddVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Front, SpeedMultiplier * (LY > 0.0f ? LY : 1.0F)));
    if (KeyboardIsKeyDown(KeyboardKey_S))
        Camera->Velocity = HMM_SubtractVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Front, SpeedMultiplier * (LY < 0.0f ? -LY : 1.0F)));
    if (KeyboardIsKeyDown(KeyboardKey_Q))
        Camera->Velocity = HMM_SubtractVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Right, SpeedMultiplier * (LX < 0.0f ? -LX : 1.0F)));
    if (KeyboardIsKeyDown(KeyboardKey_D))
        Camera->Velocity = HMM_AddVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Right, SpeedMultiplier * (LX > 0.0f ? LX : 1.0F)));

    Camera->Velocity = HMM_AddVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Front, SpeedMultiplier * LY));
    Camera->Velocity = HMM_SubtractVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Front, SpeedMultiplier * -LY));
    Camera->Velocity = HMM_AddVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Right, SpeedMultiplier * LX));
    Camera->Velocity = HMM_SubtractVec3(Camera->Velocity, HMM_MultiplyVec3f(Camera->Right, SpeedMultiplier * -LX));

    f32 FrictionMultiplier = 1.0f / (1.0f + (Camera->Friction * Delta));
    Camera->Velocity = HMM_MultiplyVec3f(Camera->Velocity, FrictionMultiplier);

    f32 VecLength = HMM_LengthVec3(Camera->Velocity);
    if (VecLength > Camera->MaxVelocity)
        Camera->Velocity = HMM_MultiplyVec3f(HMM_NormalizeVec3(Camera->Velocity), Camera->MaxVelocity);

    Camera->Position = HMM_AddVec3(Camera->Position, HMM_MultiplyVec3f(Camera->Velocity, Delta));

    i32 MX, MY = 0;
    MouseGetPosition(&MX, &MY);
    f32 DX = ((f32)MX - Camera->MousePos.X) * (CAMERA_DEFAULT_MOUSE_SENSITIVITY * Delta);
    f32 DY = ((f32)MY - Camera->MousePos.Y) * (CAMERA_DEFAULT_MOUSE_SENSITIVITY * Delta);

    if (MouseIsButtonPressed(MouseButton_Left)) {
        Camera->Yaw += DX;
        Camera->Pitch -= DY;
    }

    Camera->Yaw += RX * Delta * CAMERA_DEFAULT_GAMEPAD_SENSITIVITY;
    Camera->Pitch += RY * Delta * CAMERA_DEFAULT_GAMEPAD_SENSITIVITY;

    NoClipCameraUpdateVectors(Camera);
}

void NoClipCameraResize(noclip_camera* Camera, i32 Width, i32 Height)
{
    Camera->Width = (f32)Width;
    Camera->Height = (f32)Height;

    NoClipCameraUpdateVectors(Camera);
}

void NoClipCameraUpdateFrustum(noclip_camera* Camera)
{
    const f32 HalfVerticalSide = 10000.0f * tanf(HMM_ToRadians(75.0f) * 0.5f);
    const f32 HalfHorizontalSide = HalfVerticalSide * (Camera->Width / Camera->Height);
    const hmm_vec3 FrontMultFar = HMM_MultiplyVec3f(Camera->Front, 10000.0f);

    Camera->ViewFrustum.Near.Point = HMM_AddVec3(Camera->Position, HMM_MultiplyVec3f(Camera->Front, 0.001f));
    Camera->ViewFrustum.Near.Norm = Camera->Front;

    Camera->ViewFrustum.Far.Point = HMM_AddVec3(Camera->Position, FrontMultFar);
    Camera->ViewFrustum.Far.Norm = HMM_MultiplyVec3f(Camera->Front, -1.0f);

    Camera->ViewFrustum.Right.Point = Camera->Position;
    Camera->ViewFrustum.Right.Norm = HMM_Cross(Camera->Up, HMM_AddVec3(FrontMultFar, HMM_MultiplyVec3f(Camera->Right, HalfHorizontalSide)));

    Camera->ViewFrustum.Left.Point = Camera->Position;
    Camera->ViewFrustum.Left.Norm = HMM_Cross(HMM_SubtractVec3(FrontMultFar, HMM_MultiplyVec3f(Camera->Right, HalfHorizontalSide)), Camera->Up);

    Camera->ViewFrustum.Top.Point = Camera->Position;
    Camera->ViewFrustum.Top.Norm = HMM_Cross(Camera->Right, HMM_SubtractVec3(FrontMultFar, HMM_MultiplyVec3f(Camera->Up, HalfVerticalSide)));

    Camera->ViewFrustum.Bottom.Point = Camera->Position;
    Camera->ViewFrustum.Bottom.Norm = HMM_Cross(HMM_AddVec3(FrontMultFar, HMM_MultiplyVec3f(Camera->Up, HalfVerticalSide)), Camera->Right);

    // //

    Camera->Planes[0].XYZ = HMM_NormalizeVec3(Camera->ViewFrustum.Near.Norm);
    Camera->Planes[0].W = HMM_DotVec3(Camera->Planes[0].XYZ, Camera->ViewFrustum.Near.Point);

    Camera->Planes[1].XYZ = HMM_NormalizeVec3(Camera->ViewFrustum.Far.Norm);
    Camera->Planes[1].W = HMM_DotVec3(Camera->Planes[1].XYZ, Camera->ViewFrustum.Far.Point);

    Camera->Planes[2].XYZ = HMM_NormalizeVec3(Camera->ViewFrustum.Right.Norm);
    Camera->Planes[2].W = HMM_DotVec3(Camera->Planes[2].XYZ, Camera->ViewFrustum.Right.Point);

    Camera->Planes[3].XYZ = HMM_NormalizeVec3(Camera->ViewFrustum.Left.Norm);
    Camera->Planes[3].W = HMM_DotVec3(Camera->Planes[3].XYZ, Camera->ViewFrustum.Left.Point);

    Camera->Planes[4].XYZ = HMM_NormalizeVec3(Camera->ViewFrustum.Top.Norm);
    Camera->Planes[4].W = HMM_DotVec3(Camera->Planes[4].XYZ, Camera->ViewFrustum.Top.Point);

    Camera->Planes[5].XYZ = HMM_NormalizeVec3(Camera->ViewFrustum.Bottom.Norm);
    Camera->Planes[5].W = HMM_DotVec3(Camera->Planes[5].XYZ, Camera->ViewFrustum.Bottom.Point);
}