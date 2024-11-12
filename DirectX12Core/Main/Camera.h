#pragma once
#include <DirectXMath.h>

using namespace DirectX;
class Camera {
public:
    Camera();
    ~Camera() = default;

    void SetPosition(float x, float y, float z);
    void SetRotation(float pitch, float yaw, float roll);

    void SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ);
    void UpdateViewMatrix();

    XMMATRIX GetViewMatrix() const { return viewMatrix; }
    XMMATRIX GetProjectionMatrix() const { return projectionMatrix; }
    XMFLOAT3 GetPosition() const { return position; }

private:
    XMFLOAT3 position;
    XMFLOAT3 rotation; // pitch, yaw, roll
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
};