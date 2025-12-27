#include "Camera.h"
#include "../Core/MathConstants.h"

namespace Wiley {
	Camera::Camera()
		:isChanged(true), orientation(DirectX::XMQuaternionIdentity()), yaw(0.0f), pitch(0.0f)
	{
		ComputeDirectionVectors();
		ComputeViewMatrix();
		ComputeProjectionMatrix();
	}

	Camera::Camera(Frustum frustum)
		:frustum(frustum), isChanged(true), orientation(DirectX::XMQuaternionIdentity()), yaw(0.0f), pitch(0.0f)
	{
		ComputeDirectionVectors();
		ComputeViewMatrix();
		ComputeProjectionMatrix();
	}

	void Camera::Update(float dt)
	{
		isChanged = false;

		const float speed = 1.0f;
		if (gInput.GetKey(Key::MouseRight)) {
			float sensitivity = 0.002f;

			float mouseDX = gInput.GetMouseDeltaX();
			float mouseDY = gInput.GetMouseDeltaY();

			float yaw = mouseDX * sensitivity;
			float pitch = mouseDY * sensitivity;


			DirectX::XMVECTOR qYaw = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), yaw);

			orientation = DirectX::XMQuaternionMultiply(qYaw, orientation);

			DirectX::XMVECTOR right = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), orientation);

			DirectX::XMVECTOR qPitch = DirectX::XMQuaternionRotationAxis(right, pitch);

			orientation = DirectX::XMQuaternionMultiply(orientation, qPitch);

			orientation = DirectX::XMQuaternionNormalize(orientation);

			ComputeDirectionVectors();
			isChanged = true;
		}

		{
			if (gInput.GetKey(Key::W)) {
				frustum.position.x += frustum.target.x * speed;
				frustum.position.y += frustum.target.y * speed;
				frustum.position.z += frustum.target.z * speed;
				isChanged = true;
			}
			if (gInput.GetKey(Key::S)) {
				frustum.position.x -= frustum.target.x * speed;
				frustum.position.y -= frustum.target.y * speed;
				frustum.position.z -= frustum.target.z * speed;
				isChanged = true;
			}
			if (gInput.GetKey(Key::A)) {
				DirectX::XMVECTOR right = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), orientation);

				DirectX::XMFLOAT3 r;
				DirectX::XMStoreFloat3(&r, right);

				frustum.position.x -= r.x * speed;
				frustum.position.y -= r.y * speed;
				frustum.position.z -= r.z * speed;
				isChanged = true;
			}

			if (gInput.GetKey(Key::D)) {
				DirectX::XMVECTOR right = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), orientation);

				DirectX::XMFLOAT3 r;
				DirectX::XMStoreFloat3(&r, right);

				frustum.position.x += r.x * speed;
				frustum.position.y += r.y * speed;
				frustum.position.z += r.z * speed;
				isChanged = true;
			}
		}


		if (isChanged) {
			ComputeViewMatrix();
			ComputeProjectionMatrix();
		}
	}

	void Camera::Zoom(int increment)
	{
		//frustum.fov -= (increment * 1.0f);
		//frustum.fov = std::clamp(frustum.fov, 0.00005f, pi_div_2<float>);

		ComputeProjectionMatrix();
	}

	void Camera::OnResize(uint32_t width, uint32_t height)
	{
		SetAspectRatio(static_cast<float>(width) / height);
		isChanged = true;
	}

	void Camera::SetPosition(DirectX::XMFLOAT4 pos) {
		frustum.position = pos;
		ComputeViewMatrix();
		isChanged = true;
	}

	void Camera::SetTarget(DirectX::XMFLOAT4 tar) {
		frustum.target = tar;
		ComputeViewMatrix();
		isChanged = true;
	}

	void Camera::SetAspectRatio(float ar) {
		frustum.aspectRatio = ar;
		ComputeProjectionMatrix();
	}

	void Camera::SetFOV(float fov) {
		frustum.fov = fov;
		ComputeProjectionMatrix();
	}

	void Camera::SetNear(float near) {
		frustum.near = near;
		ComputeProjectionMatrix();
		isChanged = true;
	}

	void Camera::SetFar(float far) {
		frustum.far = far;
		ComputeProjectionMatrix();
		isChanged = true;
	}

	DirectX::XMFLOAT4 Camera::GetPosition() const
	{
		return frustum.position;
	}

	DirectX::XMFLOAT4 Camera::GetTarget() const
	{
		return frustum.target;
	}
	float Camera::GetFOV() const
	{
		return frustum.fov;
	}
	float Camera::GetAspectRatio() const
	{
		return frustum.aspectRatio;
	}
	float Camera::GetNear() const
	{
		return frustum.near;
	}
	float Camera::GetFar() const
	{
		return frustum.far;
	}

	DirectX::XMMATRIX Camera::GetView() const
	{
		return DirectX::XMMatrixTranspose(view);
	}

	DirectX::XMMATRIX Camera::GetProjection() const
	{
		return DirectX::XMMatrixTranspose(projection);
	}

	DirectX::XMMATRIX Camera::GetViewProjection() const
	{
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(view, projection));
	}

	DirectX::XMMATRIX Camera::GetInverseView() const
	{
		DirectX::XMVECTOR determinant;
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&determinant, view));;
	}

	DirectX::XMMATRIX Camera::GetInverseProjection() const
	{
		DirectX::XMVECTOR determinant;
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&determinant, projection));
	}

	bool Camera::IsChanged() const
	{
		return isChanged;
	}
	
	void Camera::ComputeDirectionVectors()
	{
		DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), orientation);
		DirectX::XMVECTOR up = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), orientation);

		XMStoreFloat4(&frustum.target, forward);
		XMStoreFloat4(&frustum.up, up);
	}

	void Camera::ComputeViewMatrix()
	{
		DirectX::XMVECTOR positionVector = DirectX::XMVectorSet(frustum.position.x, frustum.position.y, frustum.position.z, frustum.position.w);
		DirectX::XMVECTOR targetVector = DirectX::XMVectorSet(frustum.target.x, frustum.target.y, frustum.target.z, frustum.target.w);
		DirectX::XMVECTOR upVector = DirectX::XMVectorSet(frustum.up.x, frustum.up.y, frustum.up.z, frustum.up.w);

		DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(positionVector, targetVector);
		view = DirectX::XMMatrixLookAtLH(positionVector, lookAt, upVector);
	}
	void Camera::ComputeProjectionMatrix()
	{
		projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(frustum.fov), frustum.aspectRatio, frustum.near, frustum.far);
	}
}