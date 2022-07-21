#pragma once

#include "Renderer.h"

namespace orangelie
{
	class Dx12UIFont : public Renderer
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SrvHeap = nullptr;

		void BuildTexture()
		{

		}

		void BuildDescriptor()
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDescriptor = {};
			srvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			srvHeapDescriptor.NodeMask = 0;
			srvHeapDescriptor.NumDescriptors = 1;
			srvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

			HR(mDevice->CreateDescriptorHeap(&srvHeapDescriptor, IID_PPV_ARGS(SrvHeap.GetAddressOf())));

			D3D12_SHADER_RESOURCE_VIEW_DESC srViewDescriptor = {};
			srViewDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		}

	protected:
		virtual void init() override
		{
			BuildTexture();
			BuildDescriptor();


		}

		virtual void update(float dt) override
		{

		}

		virtual void draw(float dt) override
		{
			HR(mCommandAllocator->Reset());
			HR(mGraphicsCommandList->Reset(mCommandAllocator.Get(), nullptr));

			mGraphicsCommandList->ResourceBarrier(1,
				&unmove(CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffer[mCurrBackBufferIndex].Get(),
					D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

			auto rtv = rtvHandle();
			auto dsv = dsvHandle();

			FLOAT colors[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
			mGraphicsCommandList->ClearRenderTargetView(rtv, colors, 0, nullptr);
			mGraphicsCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			mGraphicsCommandList->OMSetRenderTargets(1, &rtv, true, &dsv);

			mGraphicsCommandList->RSSetScissorRects(1, &mScissorRect);
			mGraphicsCommandList->RSSetViewports(1, &mViewPort);

			mGraphicsCommandList->ResourceBarrier(1,
				&unmove(CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffer[mCurrBackBufferIndex].Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

			mGraphicsCommandList->Close();
			ID3D12CommandList* cmdLists[] = { mGraphicsCommandList.Get() };
			mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

			mSwapChain->Present(0, 0);
			mCurrBackBufferIndex = (mCurrBackBufferIndex + 1) % gBackBufferCount;

			FlushCommandList();
		}
	};
}