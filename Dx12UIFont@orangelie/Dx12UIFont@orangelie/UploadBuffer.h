#pragma once

#include "Utils.h"

namespace orangelie
{
	template<class _Tp>
	class UploadBuffer
	{
	public:
		UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : mIsConstantBuffer(isConstantBuffer)
		{
			mElementByteSize = sizeof(_Tp);

			if (isConstantBuffer)
			{
				mElementByteSize = Utils::ConstantBufferSize(sizeof(_Tp));
			}

			HR(device->CreateCommittedResource(
				&unmove(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
				D3D12_HEAP_FLAG_NONE,
				&unmove(CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount)),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(mUploadBuffer.GetAddressOf())));

			mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData));
		}

		virtual ~UploadBuffer()
		{
			if (mMappedData != nullptr)
			{
				mUploadBuffer->Unmap(0, nullptr);
				mMappedData = nullptr;
				mUploadBuffer = nullptr;
			}
		}

		UploadBuffer(const UploadBuffer&) = delete;
		UploadBuffer operator=(const UploadBuffer) = delete;

		ID3D12Resource* Resource() const
		{
			return mUploadBuffer.Get();
		}

		void CopyData(UINT elementIndex, const _Tp& data)
		{
			memcpy(&mMappedData[mElementByteSize * elementIndex], &data, sizeof(_Tp));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer = nullptr;

		BYTE* mMappedData = nullptr;
		UINT mElementByteSize;
		bool mIsConstantBuffer;

	};
}