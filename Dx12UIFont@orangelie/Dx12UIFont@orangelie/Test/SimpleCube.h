#pragma once

#if defined(ZEKROS_ENGINE)
#undef ZEKROS_ENGINE
#endif
#define ZEKROS_ENGINE orangelie::SimpleCube

#include "../Renderer.h"

namespace orangelie
{
	class SimpleCube : public Renderer
	{
	private:
		Camera mCamera;
		std::vector<std::unique_ptr<FrameResource>> mFrameResources;
		FrameResource* mCurrFrameResource = nullptr;
		UINT mCurrFrameResourceIndex = 0;

		std::unordered_map<std::string, std::unique_ptr<Shader::Texture>> mTextures;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
		std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayouts;
		std::unordered_map<std::string, std::unique_ptr<Shader::MeshGeometry>> mDrawArgs;
		std::vector<std::unique_ptr<Shader::RenderItem>> mAllRenderItems;
		std::vector<Shader::RenderItem*> mRenderLayer[(size_t)Shader::RenderLayer::Count];
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatrue = nullptr;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mGraphicsPSO;

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

			constexpr const size_t parameterSize = 3;
			CD3DX12_ROOT_PARAMETER rootParameter[parameterSize];
			rootParameter[0].InitAsConstantBufferView(0);												// Object
			rootParameter[1].InitAsConstantBufferView(1);												// Pass
			rootParameter[2].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);		// Textures

			auto staticSamplers = GetStaticSamplers();

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
			rootSignatureDescriptor.Init(parameterSize, rootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
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
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
			CopyMemory(meshGeometry->VertexCpu->GetBufferPointer(), vertices.data(), vertexBufferSize);
			HR(D3DCreateBlob(indexBufferSize, meshGeometry->IndexCpu.GetAddressOf()));
			CopyMemory(meshGeometry->IndexCpu->GetBufferPointer(), indices.data(), indexBufferSize);

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

		void BuildBoxMesh()
		{
			GeometryGenerator geoGen;
			GeometryGenerator::MeshData quad = geoGen.CreateBox(10.0f, 10.0f, 10.0f, 20);

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
			meshGeometry->name = "shapeSub";

			HR(D3DCreateBlob(vertexBufferSize, meshGeometry->VertexCpu.GetAddressOf()));
			CopyMemory(meshGeometry->VertexCpu->GetBufferPointer(), vertices.data(), vertexBufferSize);
			HR(D3DCreateBlob(indexBufferSize, meshGeometry->IndexCpu.GetAddressOf()));
			CopyMemory(meshGeometry->IndexCpu->GetBufferPointer(), indices.data(), indexBufferSize);

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

			meshGeometry->DrawArgs["box"] = subMeshGeo;

			mDrawArgs[meshGeometry->name] = std::move(meshGeometry);
		}

		void BuildRenderItems()
		{
			auto quadRitem = std::make_unique<Shader::RenderItem>();
			quadRitem->ObjIndex = 0;
			DirectX::XMStoreFloat4x4(&quadRitem->World, DirectX::XMMatrixRotationY(60.0f) * DirectX::XMMatrixTranslation(0.0f, 0.0, 50.0f));
			quadRitem->TexTransform = Utils::MatrixIdentity();
			quadRitem->meshGeo = mDrawArgs["shapeSub"].get();
			quadRitem->IndexCount = quadRitem->meshGeo->DrawArgs["box"].IndexCount;
			quadRitem->BaseVertexLocation = quadRitem->meshGeo->DrawArgs["box"].BaseVertexLocation;
			quadRitem->StartIndexLocation = quadRitem->meshGeo->DrawArgs["box"].StartIndexLocation;
			quadRitem->PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			mRenderLayer[(size_t)Shader::RenderLayer::Text].push_back(quadRitem.get());
			mAllRenderItems.push_back(std::move(quadRitem));
		}

		void BuildFrameResources()
		{
			for (int i = 0; i < Shader::gNumFrameResources; ++i)
			{
				mFrameResources.push_back(std::make_unique<FrameResource>(mDevice.Get(), 1, (UINT)mAllRenderItems.size(), 1));
			}
		}

		void BuildPSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPSODescriptor = {};
			graphicsPSODescriptor.NodeMask = 0;
			graphicsPSODescriptor.InputLayout = { mInputLayouts["layout1"].data(), (UINT)mInputLayouts["layout1"].size() };
			graphicsPSODescriptor.pRootSignature = mRootSignatrue.Get();
			graphicsPSODescriptor.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			graphicsPSODescriptor.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			graphicsPSODescriptor.DSVFormat = gDepthStencilFormat;
			graphicsPSODescriptor.NumRenderTargets = 1;
			graphicsPSODescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			graphicsPSODescriptor.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			graphicsPSODescriptor.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			graphicsPSODescriptor.RTVFormats[0] = gBackBufferFormat;
			graphicsPSODescriptor.SampleMask = UINT_MAX;
			graphicsPSODescriptor.SampleDesc = { 1, 0 };
			graphicsPSODescriptor.VS = {
				reinterpret_cast<BYTE*>(mShaders["vs"]->GetBufferPointer()),
				mShaders["vs"]->GetBufferSize()
			};
			graphicsPSODescriptor.PS = {
				reinterpret_cast<BYTE*>(mShaders["ps"]->GetBufferPointer()),
				mShaders["ps"]->GetBufferSize()
			};

			HR(mDevice->CreateGraphicsPipelineState(&graphicsPSODescriptor, IID_PPV_ARGS(mGraphicsPSO["opaque"].GetAddressOf())));
		}

		void UpdateObjectCB()
		{
			for (auto& e : mAllRenderItems)
			{
				if (e->NumframeDirty > 0)
				{
					ObjectConstants objConstants = {};
					DirectX::XMStoreFloat4x4(&objConstants.World, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&e->World)));
					DirectX::XMStoreFloat4x4(&objConstants.TexTransform, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&e->TexTransform)));

					mCurrFrameResource->mObjCB->CopyData(e->ObjIndex, objConstants);

					--(e->NumframeDirty);
				}
			}
		}

		void UpdatePassCB()
		{
			DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&mCamera.View());
			DirectX::XMMATRIX Proj = DirectX::XMLoadFloat4x4(&mCamera.Projection());
			DirectX::XMMATRIX ViewProj = DirectX::XMMatrixMultiply(View, Proj);

			PassConstants passConstants = {};
			DirectX::XMStoreFloat4x4(&passConstants.View, DirectX::XMMatrixTranspose(View));
			DirectX::XMStoreFloat4x4(&passConstants.InvView, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(View), View)));
			DirectX::XMStoreFloat4x4(&passConstants.Proj, DirectX::XMMatrixTranspose(Proj));
			DirectX::XMStoreFloat4x4(&passConstants.InvProj, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(Proj), Proj)));
			DirectX::XMStoreFloat4x4(&passConstants.ViewProj, DirectX::XMMatrixTranspose(ViewProj));
			DirectX::XMStoreFloat4x4(&passConstants.InvViewProj, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(ViewProj), ViewProj)));
			passConstants.EyePos = { 0.0f, 0.0f, 0.0f };
			passConstants.DeltaTime = mGameTimer.DeltaTime();
			passConstants.TotalTime = mGameTimer.TotalTime();

			passConstants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
			passConstants.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
			passConstants.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
			passConstants.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
			passConstants.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
			passConstants.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
			passConstants.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

			mCurrFrameResource->mPassCB->CopyData(0, passConstants);
		}

		void DrawRitems(const std::vector<Shader::RenderItem*>& rItems)
		{
			UINT cbPerObjectSize = Utils::ConstantBufferSize(sizeof(ObjectConstants));

			for (auto& r : rItems)
			{
				mGraphicsCommandList->IASetVertexBuffers(0, 1, &r->meshGeo->VertexBufferView());
				mGraphicsCommandList->IASetIndexBuffer(&r->meshGeo->IndexBufferView());
				mGraphicsCommandList->IASetPrimitiveTopology(r->PrimitiveTopology);

				mGraphicsCommandList->SetGraphicsRootConstantBufferView(0, r->ObjIndex * cbPerObjectSize + mCurrFrameResource->mObjCB->Resource()->GetGPUVirtualAddress());
				mGraphicsCommandList->DrawIndexedInstanced(r->IndexCount, 1, r->StartIndexLocation, r->BaseVertexLocation, 0);
			}
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
			BuildBoxMesh();
			BuildRenderItems();
			BuildFrameResources();
			BuildPSO();

			HR(mGraphicsCommandList->Close());
			ID3D12CommandList* cmdLists[] = { mGraphicsCommandList.Get() };
			mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

			FlushCommandList();
		}

		virtual void update(float dt) override
		{
			mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % Shader::gNumFrameResources;
			mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

			if (mFence->GetCompletedValue() < mCurrFrameResource->mFenceCount && mCurrFrameResource->mFenceCount != 0)
			{
				HANDLE hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
				mFence->SetEventOnCompletion(mCurrFrameResource->mFenceCount, hEvent);
				WaitForSingleObject(hEvent, 0xffffffff);
				CloseHandle(hEvent);
			}

			UpdateObjectCB();
			UpdatePassCB();
		}

		virtual void draw(float dt) override
		{
			HR(mCommandAllocator->Reset());
			HR(mGraphicsCommandList->Reset(mCommandAllocator.Get(), mGraphicsPSO["opaque"].Get()));

			mGraphicsCommandList->ResourceBarrier(1,
				&unmove(CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffer[mCurrBackBufferIndex].Get(),
					D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

			mGraphicsCommandList->SetGraphicsRootSignature(mRootSignatrue.Get());
			ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvHeap.Get() };
			mGraphicsCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			mGraphicsCommandList->SetGraphicsRootConstantBufferView(1, mCurrFrameResource->mPassCB->Resource()->GetGPUVirtualAddress());
			CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
			mGraphicsCommandList->SetGraphicsRootDescriptorTable(2, srvHandle);


			auto rtv = rtvHandle();
			auto dsv = dsvHandle();

			FLOAT colors[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
			mGraphicsCommandList->ClearRenderTargetView(rtv, colors, 0, nullptr);
			mGraphicsCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			mGraphicsCommandList->OMSetRenderTargets(1, &rtv, true, &dsv);

			mGraphicsCommandList->RSSetScissorRects(1, &mScissorRect);
			mGraphicsCommandList->RSSetViewports(1, &mViewPort);

			DrawRitems(mRenderLayer[(int)Shader::RenderLayer::Text]);

			mGraphicsCommandList->ResourceBarrier(1,
				&unmove(CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffer[mCurrBackBufferIndex].Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

			mGraphicsCommandList->Close();
			ID3D12CommandList* cmdLists[] = { mGraphicsCommandList.Get() };
			mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

			mSwapChain->Present(0, 0);
			mCurrBackBufferIndex = (mCurrBackBufferIndex + 1) % gBackBufferCount;

			// FlushCommandList();

			mCurrFrameResource->mFenceCount = ++mCurrFenceCount;
			mCommandQueue->Signal(mFence.Get(), mCurrFenceCount);
		}

		virtual void OnResize(UINT screenWidth, UINT screenHeight) override
		{
			Renderer::OnResize(screenWidth, screenHeight);

			mCamera.SetInstancedLookat(
				DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
				DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
				DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
			mCamera.SetLens(0.25f * DirectX::XM_PI, (float)mClientWidth / (float)mClientHeight, 1.0f, 1000.0f);
		}
	};
}