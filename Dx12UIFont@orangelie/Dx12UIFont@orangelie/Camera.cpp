#include "Camera.h"

namespace orangelie
{
	CLASSIFICATION_C1(Camera);

	void Camera::SetInstancedLookat(DirectX::XMVECTOR eyePos, DirectX::XMVECTOR lookAt, DirectX::XMVECTOR UpVec)
	{
		DirectX::XMMATRIX lookatMatrix = DirectX::XMMatrixLookAtLH(eyePos, lookAt, UpVec);
		DirectX::XMStoreFloat4x4(&mView, lookatMatrix);
	}

	void Camera::SetLens(float fovAngleY, float aspectRatio, float nearZ, float farZ)
	{
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
		DirectX::XMStoreFloat4x4(&mProjection, projectionMatrix);
	}

	DirectX::XMFLOAT4X4 Camera::View() const
	{
		return mView;
	}

	DirectX::XMFLOAT4X4 Camera::Projection() const
	{
		return mProjection;
	}
}