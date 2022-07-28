#pragma once
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <array>

#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <wincodec.h>

#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <DDSTextureLoader.h>
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <DirectXMath.h>

#define CLASSIFICATION_H(n) n(); \
n(const n&) = delete; \
n operator=(const n) = delete; \
virtual ~n();

#define CLASSIFICATION_C1(n) n::n() {} \
n::~n() {}

#define CLASSIFICATION_C2(n, e) n::n() e \
n::~n() {}

#define HR(n) { \
	if(FAILED(n)) \
	{ \
		std::string err = std::string(__FILE__) + std::string(__FUNCTION__) + std::to_string(__LINE__); \
		throw std::runtime_error(err); \
	} \
}

template <class _Tp>
_Tp& unmove(_Tp&& __value)
{
	return __value;
}

const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();


namespace Utils
{
    DirectX::XMFLOAT4X4 MatrixIdentity();

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultResource(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* data,
        size_t size,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap);

    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        const std::wstring& filename,
        const D3D_SHADER_MACRO* macro,
        const std::string& entryPoint,
        const std::string& target);
}

namespace WICConverter
{
    // get the dxgi format equivilent of a wic format
    DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);

    // get a dxgi compatible wic format from another wic format
    WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);

    // get the number of bits per pixel for a dxgi format
    int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

    // load and decode image from file
    int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);

    HRESULT CreateWICTextureFromFile12(_In_ ID3D12Device* device,
        _In_ ID3D12GraphicsCommandList* cmdList,
        _In_z_ const wchar_t* szFileName,
        _Out_ Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
        _Out_ Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap);
}

namespace Shader
{
    static const UINT gNumFrameResources = 3;


    struct SubmeshGeometry
    {
        UINT IndexCount;
        UINT StartIndexLocation;
        INT BaseVertexLocation;
    };

    struct MeshGeometry
    {
        std::string name;

        Microsoft::WRL::ComPtr<ID3DBlob> VertexCpu, IndexCpu;
        Microsoft::WRL::ComPtr<ID3D12Resource> VertexGpu, IndexGpu;
        Microsoft::WRL::ComPtr<ID3D12Resource> VertexGpuUploader, IndexGpuUploader;

        std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

        UINT VertexByteStride;
        UINT VertexBufferByteSize;

        DXGI_FORMAT IndexFormat;
        UINT IndexBufferByteSize;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
        {
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
            vertexBufferView.BufferLocation = VertexGpu->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = VertexBufferByteSize;
            vertexBufferView.StrideInBytes = VertexByteStride;

            return vertexBufferView;
        }

        D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
        {
            D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
            indexBufferView.BufferLocation = IndexGpu->GetGPUVirtualAddress();
            indexBufferView.Format = IndexFormat;
            indexBufferView.SizeInBytes = IndexBufferByteSize;

            return indexBufferView;
        }
    };

    struct RenderItem
    {
        RenderItem() = default;

        DirectX::XMFLOAT4X4 World = Utils::MatrixIdentity();
        DirectX::XMFLOAT4X4 TexTransform = Utils::MatrixIdentity();

        UINT NumframeDirty = gNumFrameResources;

        MeshGeometry* meshGeo = nullptr;

        UINT IndexCount;
        UINT StartIndexLocation;
        INT BaseVertexLocation;

        D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    };

    struct Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT3 Tangent;
        DirectX::XMFLOAT2 TexCoord;
    };

    struct Texture
    {
        std::string name;
        std::wstring fileName;

        Microsoft::WRL::ComPtr<ID3D12Resource> ResourceGpuHeap;
        Microsoft::WRL::ComPtr<ID3D12Resource> UploadGpuHeap;
    };

    struct Material
    {
        DirectX::XMFLOAT4 R0;
        DirectX::XMFLOAT3 DiffuseAlbedo;
        float Roughness;
    };
}