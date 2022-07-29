#include "FrameResource.h"

namespace orangelie
{
	FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objCount, UINT matCount)
	{
		HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocator.GetAddressOf())));

		mObjCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objCount, true);
		mPassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
		mMatVB = std::make_unique<UploadBuffer<MaterialConstants>>(device, matCount, false);
	}

	FrameResource::~FrameResource()
	{
	}
}