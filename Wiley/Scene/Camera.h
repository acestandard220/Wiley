#pragma once
#include "../Core/Input.h"

#include <DirectXMath.h>


#undef near
#undef far

namespace Wiley {

	struct Frustum {
		float fov;
		float aspectRatio;
		float near;
		float far;
		
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 target;
		DirectX::XMFLOAT4 up;

		Frustum()
			:fov(45.0f), near(0.1f), far(1000.0f), aspectRatio(1.0f),
			position({ 0.0f,2.0f,-8.0f,1.0f }), target({ 0.0f,0.0f,1.0f,1.0f }), up({ 0.0f,1.0f,0.0f,1.0f })
		{
		}
	};

	/// <summary>
	///		This class works internaly with row_major but all returns are made to column_major
	/// </summary>
	class Camera {
		public:
			using Ref = std::shared_ptr<Camera>;

			Camera();
			explicit Camera(Frustum frustum);
			~Camera() = default;

			void Update(float dt);
			void Zoom(int increment);
			void OnResize(uint32_t width, uint32_t height);

			void SetPosition(DirectX::XMFLOAT4 pos);
			void SetTarget(DirectX::XMFLOAT4 tar);

			void SetFOV(float fov);
			void SetAspectRatio(float aspectRatio);
			void SetNear(float near);
			void SetFar(float far);

			DirectX::XMFLOAT4 GetPosition()const;
			DirectX::XMFLOAT4 GetTarget()const;

			float GetFOV()const;
			float GetAspectRatio()const;
			float GetNear()const;
			float GetFar()const;

			/// <summary>
			///		Returns the view matrix of the camera in column major.
			/// </summary>
			DirectX::XMMATRIX GetView()const;

			/// <summary>
			///		Returns the projection matrix of the camera in column major.
			/// </summary>
			DirectX::XMMATRIX GetProjection()const;

			/// <summary>
			///		Returns the view projection matrix of the camera in column major.
			/// </summary>
			DirectX::XMMATRIX GetViewProjection()const;

			/// <summary>
			///		Returns the inverse view matrix of the camera in column major.
			/// </summary>
			DirectX::XMMATRIX GetInverseView()const;

			/// <summary>
			///		Returns the inverse projection matrix of the camera in column major.
			/// </summary>
			DirectX::XMMATRIX GetInverseProjection()const;

			DirectX::XMMATRIX GetInverseViewProjection()const;

			bool IsChanged()const;
		private:
			void ComputeDirectionVectors();
			void ComputeViewMatrix();
			void ComputeProjectionMatrix();
		private:
			Frustum frustum;
			float yaw;
			float pitch;
			float row;
			DirectX::XMVECTOR orientation;

			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;

			bool isChanged;
	};

}