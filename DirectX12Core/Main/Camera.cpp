#include "Camera.h"
using namespace DirectX;

Camera::Camera()
    : position{ 0.0f, 2.0f, -5.0f }, rotation{ 0.0f, 0.0f, 0.0f } 
{
    SetPerspective(XM_PIDIV4, 800.0f / 600.0f, 0.1f, 100.0f);
    UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z) 
{
    position = { x, y, z };
    UpdateViewMatrix();
}

void Camera::SetRotation(float pitch, float yaw, float roll) 
{
    rotation = { pitch, yaw, roll };
    UpdateViewMatrix();
}

void Camera::SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ) 
{
    projectionMatrix = XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ);
}

void Camera::UpdateViewMatrix() 
{
    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR lookAt = XMVectorAdd(pos, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    viewMatrix = XMMatrixLookAtLH(pos, lookAt, up);
}