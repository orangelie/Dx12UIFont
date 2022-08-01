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

	void Camera::SetPosition(float x, float y, float z)
	{
		mPosition = { x, y, z };
	}

	void Camera::LookAt(DirectX::XMFLOAT3& position, DirectX::XMFLOAT3& target, DirectX::XMFLOAT3& up)
	{
		DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&position);
		DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&target);
		DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&up);

		LookAt(P, T, U);
	}

	void Camera::LookAt(DirectX::FXMVECTOR position, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up)
	{
		DirectX::XMVECTOR L = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, position));
		DirectX::XMVECTOR R = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(L, up));
		DirectX::XMVECTOR U = DirectX::XMVector3Cross(L, R);

		DirectX::XMStoreFloat3(&mPosition, position);
		DirectX::XMStoreFloat3(&mLookAt, L);
		DirectX::XMStoreFloat3(&mRight, R);
		DirectX::XMStoreFloat3(&mUp, U);

		mViewDirty = true;
	}

	void Camera::UpdateViewMatrix()
	{
		if (mViewDirty)
		{
			DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&mPosition);
			DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&mLookAt);
			DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&mUp);
			DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&mRight);

			L = DirectX::XMVector3Normalize(L);
			U = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(L, R));
			R = DirectX::XMVector3Cross(U, L);

			float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, R));
			float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, U));
			float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, L));

			DirectX::XMStoreFloat3(&mPosition, P);
			DirectX::XMStoreFloat3(&mRight, R);
			DirectX::XMStoreFloat3(&mUp, U);
			DirectX::XMStoreFloat3(&mLookAt, L);

			mView.m[0][0] = mRight.x;	mView.m[0][1] = mUp.x;	mView.m[0][2] = mLookAt.x;	mView.m[0][3] = 0.0f;
			mView.m[1][0] = mRight.y;	mView.m[1][1] = mUp.y;	mView.m[1][2] = mLookAt.y;	mView.m[1][3] = 0.0f;
			mView.m[2][0] = mRight.z;	mView.m[2][1] = mUp.z;	mView.m[2][2] = mLookAt.z;	mView.m[2][3] = 0.0f;
			mView.m[3][0] = x;			mView.m[3][1] = y;		mView.m[3][2] = z;			mView.m[3][3] = 1.0f;

			mViewDirty = false;
		}
	}

	void Camera::Strafe(float d)
	{
		DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&mPosition);
		DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&mRight);
		DirectX::XMVECTOR D = DirectX::XMVectorReplicate(d);

		DirectX::XMStoreFloat3(&mPosition, DirectX::XMVectorMultiplyAdd(D, R, P));
		mViewDirty = true;
	}

	void Camera::Walk(float d)
	{
		DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&mPosition);
		DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&mLookAt);
		DirectX::XMVECTOR D = DirectX::XMVectorReplicate(d);

		DirectX::XMStoreFloat3(&mPosition, DirectX::XMVectorMultiplyAdd(D, L, P));
		mViewDirty = true;
	}

	void Camera::Pitch(float d)
	{
		DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&mRight);
		DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&mUp);
		DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&mLookAt);
		DirectX::XMMATRIX rot = DirectX::XMMatrixRotationAxis(R, d);

		DirectX::XMStoreFloat3(&mUp, DirectX::XMVector3TransformNormal(U, rot));
		DirectX::XMStoreFloat3(&mLookAt, DirectX::XMVector3TransformNormal(L, rot));
		mViewDirty = true;
	}

	void Camera::RotationY(float d)
	{
		DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&mRight);
		DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&mUp);
		DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&mLookAt);
		DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(d);

		DirectX::XMStoreFloat3(&mRight, DirectX::XMVector3TransformNormal(R, rot));
		DirectX::XMStoreFloat3(&mUp, DirectX::XMVector3TransformNormal(U, rot));
		DirectX::XMStoreFloat3(&mLookAt, DirectX::XMVector3TransformNormal(L, rot));
		mViewDirty = true;
	}

	DirectX::XMFLOAT4X4 Camera::View() const
	{
		return mView;
	}

	DirectX::XMFLOAT4X4 Camera::Projection() const
	{
		return mProjection;
	}

	DirectX::XMFLOAT3 Camera::Position() const
	{
		return mPosition;
	}
}