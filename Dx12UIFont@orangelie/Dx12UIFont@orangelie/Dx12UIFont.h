#pragma once

#include "Renderer.h"

namespace orangelie
{
	class Dx12UIFont : public Renderer
	{
	private:
		std::unordered_map<std::string, std::unique_ptr<Shader::Texture>> mTextures;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
		std::unordered_map<std::string, D3D12_INPUT_ELEMENT_DESC> mInputLayouts;
		std::unordered_map<std::string, std::unique_ptr<Shader::MeshGeometry>> mDrawArgs;
		std::vector<std::unique_ptr<Shader::RenderItem>> mAllRenderItems;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatrue = nullptr;

		void BuildTexture()
		{
			auto tempTexture = std::make_unique<Shader::Texture>();
			tempTexture->name = "text";
			tempTexture->fileName = L"Text.png";

			HR(WICConverter::CreateWICTextureFromFile12(mDevice.Get(),
				mGraphicsCommandList.Get(),
				tempTexture->fileName.c_str(),
				tempTexture->ResourceGpuHeap,
				tempTexture->UploadGpuHeap));

			mTextures[tempTexture->name] = std::move(tempTexture);

			/*
			HR(DirectX::CreateDDSTextureFromFile12(mDevice.Get(),
				mGraphicsCommandList.Get(),
				tempTexture->fileName.c_str(),
				tempTexture->ResourceGpuHeap,
				tempTexture->UploadGpuHeap));
			*/
		}

		void BuildRootSignature()
		{
			CD3DX12_DESCRIPTOR_RANGE srvRange;
			srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

			CD3DX12_ROOT_PARAMETER rootParameter;
			rootParameter.InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

			auto staticSamplers = GetStaticSamplers();

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
			rootSignatureDescriptor.Init(1, &rootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			Microsoft::WRL::ComPtr<ID3DBlob> ppBlob, ppErrorMsg;
			HR(D3D12SerializeRootSignature(&rootSignatureDescriptor,
				D3D_ROOT_SIGNATURE_VERSION_1, ppBlob.GetAddressOf(), ppErrorMsg.GetAddressOf()));

			if (ppErrorMsg != nullptr)
			{
				MessageBoxA(0, (char*)ppErrorMsg->GetBufferPointer(), "RootSignature Error", MB_OK);
				throw std::runtime_error("RootSignature Throws");
			}

			HR(mDevice->CreateRootSignature(0,
				ppBlob->GetBufferPointer(), ppBlob->GetBufferSize(), IID_PPV_ARGS(mRootSignatrue.GetAddressOf())));
		}
		 
		void BuildDescriptor()
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDescriptor = {};
			srvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			srvHeapDescriptor.NodeMask = 0;
			srvHeapDescriptor.NumDescriptors = 1;
			srvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

			HR(mDevice->CreateDescriptorHeap(&srvHeapDescriptor, IID_PPV_ARGS(mSrvHeap.GetAddressOf())));
			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(mSrvHeap->GetCPUDescriptorHandleForHeapStart());

			D3D12_SHADER_RESOURCE_VIEW_DESC srViewDescriptor = {};
			srViewDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srViewDescriptor.Texture2D.MostDetailedMip = 0;
			srViewDescriptor.Texture2D.ResourceMinLODClamp = 0.0f;
			
			auto textTexture = mTextures["text"].get()->ResourceGpuHeap;
			srViewDescriptor.Format = textTexture->GetDesc().Format;
			srViewDescriptor.Texture2D.MipLevels = textTexture->GetDesc().MipLevels;

			mDevice->CreateShaderResourceView(textTexture.Get(), &srViewDescriptor, srvHandle);
			srvHandle.Offset(1, mCbvSrvUavSize);
		}

		void BuildShadersAndInputLayout()
		{
			D3D_SHADER_MACRO shaderMacro[] =
			{
				"SHADER", "0",
				NULL, NULL
			};

			mShaders["vs"] = Utils::CompileShader(L"shader.hlsl", shaderMacro, "VS", "vs_5_1");
			mShaders["ps"] = Utils::CompileShader(L"shader.hlsl", shaderMacro, "PS", "ps_5_1");

			mInputLayouts["layout1"] = {
				"POSITION", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			};
		}

		void BuildQuadMesh()
		{
			GeometryGenerator geoGen;
			GeometryGenerator::MeshData quad = geoGen.CreateQuad(100.0f, 100.0f, 50.0f, 50.0f, 1.0f);

			std::vector<Shader::Vertex> vertices(quad.Vertices.size());
			for (size_t i = 0; i < vertices.size(); ++i)
			{
				vertices[i].Position = quad.Vertices[i].Position;
				vertices[i].Normal = quad.Vertices[i].Normal;
				vertices[i].Tangent = quad.Vertices[i].Tangent;
				vertices[i].TexCoord = quad.Vertices[i].TexCoord;
			}

			std::vector<std::uint32_t> indices = quad.Indices;

			UINT vertexBufferSize = sizeof(Shader::Vertex) * (UINT)vertices.size();
			UINT indexBufferSize = sizeof(std::uint32_t) * (UINT)indices.size();

			auto meshGeometry = std::make_unique<Shader::MeshGeometry>();
			meshGeometry->name = "shape";

			HR(D3DCreateBlob(vertexBufferSize, meshGeometry->VertexCpu.GetAddressOf()));
			HR(D3DCreateBlob(indexBufferSize, meshGeometry->IndexCpu.GetAddressOf()));
			meshGeometry->VertexGpu = Utils::CreateDefaultResource(mDevice.Get(), mGraphicsCommandList.Get(),
				vertices.data(), vertexBufferSize, meshGeometry->VertexGpuUploader);
			meshGeometry->IndexGpu = Utils::CreateDefaultResource(mDevice.Get(), mGraphicsCommandList.Get(),
				indices.data(), indexBufferSize, meshGeometry->IndexGpuUploader);

			meshGeometry->VertexBufferByteSize = vertexBufferSize;
			meshGeometry->VertexByteStride = sizeof(Shader::Vertex);

			meshGeometry->IndexFormat = DXGI_FORMAT_R32_UINT;
			meshGeometry->IndexBufferByteSize = indexBufferSize;

			Shader::SubmeshGeometry subMeshGeo = {};
			subMeshGeo.IndexCount = (UINT)indices.size();
			subMeshGeo.BaseVertexLocation = 0;
			subMeshGeo.StartIndexLocation = 0;

			meshGeometry->DrawArgs["quad"] = subMeshGeo;

			mDrawArgs[meshGeometry->name] = std::move(meshGeometry);
		}

		void BuildRenderItems()
		{
			auto quadRitem = std::make_unique<Shader::RenderItem>();
			quadRitem->World = Utils::MatrixIdentity();
			quadRitem->TexTransform = Utils::MatrixIdentity();
			quadRitem->meshGeo = mDrawArgs["shape"].get();
			quadRitem->IndexCount = quadRitem->meshGeo->DrawArgs["quad"].IndexCount;
			quadRitem->BaseVertexLocation = quadRitem->meshGeo->DrawArgs["quad"].BaseVertexLocation;
			quadRitem->StartIndexLocation = quadRitem->meshGeo->DrawArgs["quad"].StartIndexLocation;
			quadRitem->PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			mAllRenderItems.push_back(std::move(quadRitem));
		}

	protected:
		virtual void init() override
		{
			HR(mGraphicsCommandList->Reset(mCommandAllocator.Get(), nullptr));

			BuildTexture();
			BuildRootSignature();
			BuildDescriptor();
			BuildShadersAndInputLayout();
			BuildQuadMesh();
			BuildRenderItems();

			HR(mGraphicsCommandList->Close());
			ID3D12CommandList* cmdLists[] = { mGraphicsCommandList.Get() };
			mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

			FlushCommandList();
		}

		virtual void update(float dt) override
		{

		}

		virtual void draw(float dt) override
		{
			HR(mCommandAllocator->Reset());
			HR(mGraphicsCommandList->Reset(mCommandAllocator.Get(), nullptr));

			mGraphicsCommandList->SetGraphicsRootSignature(mRootSignatrue.Get());

			ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvHeap.Get() };
			mGraphicsCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

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