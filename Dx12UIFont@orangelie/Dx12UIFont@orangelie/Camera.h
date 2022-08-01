#pragma once

#include "Utils.h"

namespace orangelie
{
	class Camera
	{
	public:
		CLASSIFICATION_H(Camera);

		void SetInstancedLookat(DirectX::XMVECTOR eyePos, DirectX::XMVECTOR lookAt, DirectX::XMVECTOR UpVec);
		void SetLens(float fovAngleY, float aspectRatio, float nearZ, float farZ);
		void SetPosition(float x, float y, float z);

		void LookAt(DirectX::XMFLOAT3& position, DirectX::XMFLOAT3& target, DirectX::XMFLOAT3& up);
		void LookAt(DirectX::FXMVECTOR position, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up);
		void UpdateViewMatrix();

		void Strafe(float d);
		void Walk(float d);
		void Pitch(float d);
		void RotationY(float d);

		DirectX::XMFLOAT4X4 View() const;
		DirectX::XMFLOAT4X4 Projection() const;
		DirectX::XMFLOAT3 Position() const;

	private:
		DirectX::XMFLOAT4X4 mProjection, mView;
		DirectX::XMFLOAT3
			mPosition = { 0.0f, 0.0f, 0.0f },
			mLookAt = { 0.0f, 0.0f, 1.0f },
			mRight = { 1.0f, 0.0f, 0.0f },
			mUp = { 0.0f, 1.0f, 0.0f };
		bool mViewDirty;

	};
}