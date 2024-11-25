#pragma once
class Camera
{
public:
	Camera();
	Camera(const XMFLOAT3& position, const XMFLOAT3& target, const XMFLOAT3& upVector);
	~Camera() = default;

	XMMATRIX& GetViewMatrix()
	{
		return this->viewMatrix;
	}
	XMMATRIX& GetProjectionMatrix()
	{
		return this->projectionMatrix;
	}

	void SetPosition(const XMFLOAT3& position)
	{
		this->position = position;
		UpdateViewMatrix();
	}

	void Move(const XMFLOAT3& direction, float deltaTime);
	void Rotate(float deltaYaw, float deltaPitch);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

private:

	XMFLOAT3 position;
	XMFLOAT3 target; // LookAt
	XMFLOAT3 upVector;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	float yaw = 0.f;
	float pitch = 0.f;
	float moveSpeed = 5.f;
	float turnSpeed = 0.1f;
};

