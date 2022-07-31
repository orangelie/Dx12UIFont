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

		DirectX::XMFLOAT4X4 View() const;
		DirectX::XMFLOAT4X4 Projection() const;

	private:
		DirectX::XMFLOAT4X4 mProjection, mView;

	};
}