#pragma once

#include "Utils.h"
#include "UploadBuffer.h"

#if defined(MAXLIGHTS)
#undef MAXLIGHTS
#endif

#define MAXLIGHTS 16

namespace orangelie
{
	struct ObjectConstants
	{
		DirectX::XMFLOAT4X4 World = Utils::MatrixIdentity();
		DirectX::XMFLOAT4X4 TexTransform = Utils::MatrixIdentity();

		UINT MatIndex;
		DirectX::XMFLOAT3 cbPerPadding1;
	};

	struct PassConstants
	{
		DirectX::XMFLOAT4X4 View = Utils::MatrixIdentity();
		DirectX::XMFLOAT4X4 InvView = Utils::MatrixIdentity();
		DirectX::XMFLOAT4X4 Proj = Utils::MatrixIdentity();
		DirectX::XMFLOAT4X4 InvProj = Utils::MatrixIdentity();
		DirectX::XMFLOAT4X4 ViewProj = Utils::MatrixIdentity();
		DirectX::XMFLOAT4X4 InvViewProj = Utils::MatrixIdentity();

		DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 EyePos;
		float TotalTime;

		Shader::Light Lights[MAXLIGHTS];

		float DeltaTime;
		DirectX::XMFLOAT3 cbPerObjectPadding1;
	};

	struct MaterialConstants
	{
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness;

		DirectX::XMFLOAT4X4 MatTransform = Utils::MatrixIdentity();

		INT DiffuseMapIndex;
		INT NormalMapHeapIndex;
		INT cbPerMaterialPadding1;
		INT cbPerMaterialPadding2;
	};

	class FrameResource
	{
	public:
		FrameResource(ID3D12Device* device, UINT passCount, UINT objCount, UINT matCount);
		FrameResource(const FrameResource&) = delete;
		FrameResource operator=(const FrameResource&) = delete;
		virtual ~FrameResource();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator = nullptr;
		UINT64 mFenceCount = 0;

		std::unique_ptr<UploadBuffer<ObjectConstants>> mObjCB = nullptr;
		std::unique_ptr<UploadBuffer<PassConstants>> mPassCB = nullptr;
		std::unique_ptr<UploadBuffer<MaterialConstants>> mMatVB = nullptr;

	};
}