#include "framework.h"
#include "Camera.h"

Camera::Camera()
{
	position = { 0.f, 0.f, 0.f };
	target = { 0.f, 0.f, 0.f };
	upVector = { 0.f, 1.f, 0.f };

	viewMatrix = XMMatrixLookAtLH(
		XMVectorSet(position.x, position.y, position.z, 1.0f),
		XMVectorSet(target.x, target.y, target.z, 1.0f),
		XMVectorSet(upVector.x, upVector.y, upVector.z, 0.0f)
	);

	projectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),
		ASPECTRATIO,
		0.1f,
		100.0f
	);
}

Camera::Camera(const XMFLOAT3& position, const XMFLOAT3& target, const XMFLOAT3& upVector)
	:position(position), target(target), upVector(upVector)
{
	viewMatrix = XMMatrixLookAtLH(
		XMVectorSet(position.x, position.y, position.z, 1.0f),
		XMVectorSet(target.x, target.y, target.z, 1.0f),
		XMVectorSet(upVector.x, upVector.y, upVector.z, 0.0f)
	);

	projectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),
		ASPECTRATIO,
		0.1f,
		100.0f
	);
}

void Camera::Move(const XMFLOAT3& direction, float deltaTime)
{
	XMVECTOR forward = XMVector3Normalize(XMLoadFloat3(&target) - XMLoadFloat3(&position));
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&upVector), forward));

	XMVECTOR movement = XMVectorZero();
	movement += XMVectorScale(forward, direction.z);
	movement += XMVectorScale(right, direction.x);
	movement += XMVectorScale(XMLoadFloat3(&upVector), direction.y);

	XMVECTOR newPosition = XMLoadFloat3(&position) + movement * moveSpeed * deltaTime;
	XMStoreFloat3(&position, newPosition);

	UpdateViewMatrix();
}

void Camera::Rotate(float deltaYaw, float deltaPitch)
{
	yaw += deltaYaw * turnSpeed;
	pitch -= deltaPitch * turnSpeed;

	pitch = clamp(pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

	XMVECTOR forward = XMVectorSet(
		cosf(pitch) * sinf(yaw),
		sinf(pitch),
		cosf(pitch) * cosf(yaw),
		0.0f
	);

	XMStoreFloat3(&target, XMLoadFloat3(&position) + forward);
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMVECTOR positionVec = XMLoadFloat3(&position);
	XMVECTOR targetVec = XMLoadFloat3(&target);
	XMVECTOR upVec = XMLoadFloat3(&upVector);

	viewMatrix = XMMatrixLookAtLH(positionVec, targetVec, upVec);
}

void Camera::UpdateProjectionMatrix()
{

}
