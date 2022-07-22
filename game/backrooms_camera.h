#pragma once

#include "backrooms_common.h"

#include <hmm/HandmadeMath.h>

#define CAMERA_DEFAULT_YAW -90.0f
#define CAMERA_DEFAULT_PITCH 0.0f
#define CAMERA_DEFAULT_SPEED 1.0f
#define CAMERA_DEFAULT_MOUSE_SENSITIVITY 5.0f
#define CAMERA_DEFAULT_GAMEPAD_SENSITIVITY 100.0f
#define CAMERA_DEFAULT_ZOOM 90.0f

struct frustum_plane
{
    hmm_vec3 Point;
    hmm_vec3 Norm;
};

struct frustum_view
{
    frustum_plane Near;
    frustum_plane Far;
    frustum_plane Right;
    frustum_plane Left;
    frustum_plane Top;
    frustum_plane Bottom;
};

// No, this is not related to how you noclip out of reality to get to Level 0. This is a noclip style camera.
struct noclip_camera
{
    f32 Yaw;
    f32 Pitch;

    hmm_vec3 Position;
    hmm_vec3 Front;
    hmm_vec3 Up;
    hmm_vec3 Right;
    hmm_vec3 WorldUp;
    
    hmm_vec3 MousePos;
    bool FirstMouse;

    hmm_mat4 View;
    hmm_mat4 Projection;

    f32 Width;
    f32 Height;

    f32 Acceleration;
    f32 Friction;
    hmm_vec3 Velocity;
    f32 MaxVelocity;

    frustum_view ViewFrustum;
    hmm_vec4 Planes[6];
};

void NoClipCameraInit(noclip_camera* Camera);
void NoClipCameraUpdate(noclip_camera* Camera, f32 Delta);
void NoClipCameraInput(noclip_camera* Camera, f32 Delta);
void NoClipCameraResize(noclip_camera* Camera, i32 Width, i32 Height);
void NoClipCameraUpdateFrustum(noclip_camera* Camera);