#pragma once

#include "Utils.h"

namespace orangelie
{
	struct PassConstants
	{

	};

	class FrameResource
	{
	public:
		CLASSIFICATION_H(FrameResource);
		FrameResource(ID3D12Device* device, UINT passCount, UINT objCount, UINT matCount);

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator = nullptr;
		UINT64 mFenceCount = 0;

		std::unique_ptr<PassConstants> mPassCB;

	};
}