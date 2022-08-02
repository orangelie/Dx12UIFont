#pragma once

#include "Utils.h"
#include "GameTimer.h"
#include "GeometryGenerator.h"
#include "Test/FrameResource/Dx12UIFont/FrameResource.h"
#include "Test/FrameResource/SimpleCube/FrameResource.h"
#include "Camera.h"
#include "Font.h"


namespace orangelie
{
	class Renderer
	{
	public:
		static Renderer* gGameApp;

		CLASSIFICATION_H(Renderer);
		virtual LRESULT MessageHandler(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam);

		void Initialize(HINSTANCE hInstance, UINT screenWidth, UINT screenHeight);
		void Render();

	private:
		void BuildWindows(HINSTANCE hInstance, UINT screenWidth, UINT screenHeight);
		void BuildDxgiAndD3D12(UINT screenWidth, UINT screenHeight);

	private:
		LPCWSTR mWndClassName = L"orangelieApp";

		HINSTANCE mModuleHandle;
		UINT mFullscreenWidth, mFullscreenHeight;
		bool mIsResizing = false, mIsMinimized = false, mIsMaximized = false, mIsEnginePaused = false;

		
		Microsoft::WRL::ComPtr<IDXGIFactory4> mFactory4 = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvDescriptorHeap = nullptr;

	protected:
		GameTimer mGameTimer;

		const static UINT gBackBufferCount = 2;
		const static DXGI_FORMAT gBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		const static DXGI_FORMAT gDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		UINT64 mCurrFenceCount = 0;
		UINT mRtvSize, mDsvSize, mCbvSrvUavSize;
		UINT mClientWidth, mClientHeight;
		HWND mHwnd;

		void FlushCommandList();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle();

		virtual void init() = 0;
		virtual void update(float dt) = 0;
		virtual void draw(float dt) = 0;
		virtual void OnResize(UINT screenWidth, UINT screenHeight);
		virtual void RButtonDown(WPARAM btnState, int x, int y) = 0;
		virtual void RButtonUp(WPARAM btnState, int x, int y) = 0;
		virtual void MouseMove(WPARAM btnState, int x, int y) = 0;

		Microsoft::WRL::ComPtr<ID3D12Device> mDevice = nullptr;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence = nullptr;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator = nullptr;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue = nullptr;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mGraphicsCommandList = nullptr;

		UINT mCurrBackBufferIndex;
		Microsoft::WRL::ComPtr<ID3D12Resource> mBackBuffer[gBackBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer = nullptr;

		D3D12_RECT mScissorRect;
		D3D12_VIEWPORT mViewPort;

	};
}

LRESULT __stdcall WindowProcedure(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam);