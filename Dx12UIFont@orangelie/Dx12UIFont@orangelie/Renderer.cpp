#include "Renderer.h"

namespace orangelie
{
	Renderer* Renderer::gGameApp = nullptr;

	CLASSIFICATION_C2(Renderer,
		{
			gGameApp = this;
		});

	LRESULT Renderer::MessageHandler(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (hMessage)
		{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				mIsEnginePaused = true;
				mGameTimer.Stop();
			}
			else {
				mIsEnginePaused = false;
				mGameTimer.Start();
			}
			return 0;

		case WM_SIZE:
			mClientWidth = LOWORD(lParam);
			mClientHeight = HIWORD(lParam);

			if (wParam == SIZE_MINIMIZED)
			{
				mIsMinimized = true;
				mIsMaximized = false;
				mIsEnginePaused = true;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mIsMinimized = false;
				mIsMaximized = true;
				mIsEnginePaused = false;
				OnResize(mClientWidth, mClientHeight);
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (mIsMinimized)
				{
					mIsMinimized = false;
					mIsEnginePaused = false;
					OnResize(mClientWidth, mClientHeight);
				}
				else if(mIsMaximized)
				{
					mIsMaximized = false;
					mIsEnginePaused = false;
					OnResize(mClientWidth, mClientHeight);
				}
				else if (mIsResizing)
				{

				}
				else
				{
					if(mDevice != nullptr)
						OnResize(mClientWidth, mClientHeight);
				}
			}

			return 0;

		case WM_MENUCHAR:
			return MAKELRESULT(0, MNC_CLOSE);

		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
				PostQuitMessage(0);
			return 0;

		case WM_ENTERSIZEMOVE:
			mGameTimer.Stop();
			mIsResizing = true;
			mIsEnginePaused = true;
			return 0;

		case WM_EXITSIZEMOVE:
			mGameTimer.Start();
			mIsResizing = false;
			mIsEnginePaused = false;
			OnResize(mClientWidth, mClientHeight);
			return 0;

		case WM_MOUSEMOVE:
			MouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_RBUTTONDOWN:
			RButtonDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_RBUTTONUP:
			RButtonUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_DESTROY: case WM_CLOSE:
			PostQuitMessage(0); return 0;
		}

		return DefWindowProcW(hWnd, hMessage, wParam, lParam);
	}

	void Renderer::Initialize(HINSTANCE hInstance, UINT screenWidth, UINT screenHeight)
	{

		/*
#if defined(DEBUG) || defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug> DebugLayer;
		HR(D3D12GetDebugInterface(IID_PPV_ARGS(DebugLayer.GetAddressOf())));
		DebugLayer->EnableDebugLayer();
#endif
		*/

		BuildWindows(hInstance, screenWidth, screenHeight);
		BuildDxgiAndD3D12(screenWidth, screenHeight);
		init();
	}

	void Renderer::Render()
	{
		MSG msg = {};
		mGameTimer.Reset();

		for (;msg.message != WM_QUIT;)
		{
			if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
			{
				mGameTimer.Tick();

				if (!mIsEnginePaused)
				{
					update(mGameTimer.DeltaTime());
					draw(mGameTimer.DeltaTime());
				}
				else
				{
					Sleep(100);
				}
			}
		}
	}

	void Renderer::BuildWindows(HINSTANCE hInstance, UINT screenWidth, UINT screenHeight)
	{
		mModuleHandle = hInstance;

		mClientWidth = screenWidth;
		mClientHeight = screenHeight;
		mFullscreenWidth = GetSystemMetrics(SM_CXSCREEN);
		mFullscreenHeight = GetSystemMetrics(SM_CYSCREEN);
		
		WNDCLASSEXW wndClassEx = {};
		wndClassEx.cbClsExtra = 0;
		wndClassEx.cbSize = sizeof(WNDCLASSEXW);
		wndClassEx.cbWndExtra = 0;
		wndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClassEx.hCursor = LoadCursorW(mModuleHandle, MAKEINTRESOURCEW(32512));
		wndClassEx.hIcon = LoadIconW(mModuleHandle, MAKEINTRESOURCEW(32512));
		wndClassEx.hIconSm = wndClassEx.hIcon;
		wndClassEx.hInstance = mModuleHandle;
		wndClassEx.lpszClassName = mWndClassName;
		wndClassEx.lpfnWndProc = WindowProcedure;
		wndClassEx.lpszMenuName = nullptr;
		wndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		RegisterClassExW(&wndClassEx);

		DEVMODEW devMode = {};
		devMode.dmBitsPerPel = 32;
		devMode.dmPelsWidth = screenWidth;
		devMode.dmPelsHeight = screenHeight;
		devMode.dmFields = DM_BITSPERPEL | DM_PELSHEIGHT | DM_PELSWIDTH;

		ChangeDisplaySettingsW(&devMode, 0);

		mHwnd = CreateWindowExW(
			0,
			mWndClassName,
			mWndClassName,
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			(INT)((float)(mFullscreenWidth - screenWidth) / 2),
			(INT)((float)(mFullscreenHeight - screenHeight) / 2),
			screenWidth,
			screenHeight,
			nullptr,
			nullptr,
			mModuleHandle,
			nullptr);

		if (mHwnd == nullptr)
		{
			throw std::runtime_error("void Renderer::BuildWindows(UINT screenWidth, UINT screenHeight);");
		}

		ShowWindow(mHwnd, SW_SHOW);
		SetForegroundWindow(mHwnd);
		ShowCursor(TRUE);
		UpdateWindow(mHwnd);
	}

	void Renderer::BuildDxgiAndD3D12(UINT screenWidth, UINT screenHeight)
	{
		// < Factory4 >
		HR(CreateDXGIFactory1(IID_PPV_ARGS(mFactory4.GetAddressOf())));

		// < Device >
		HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(mDevice.GetAddressOf())));

		// < Command Objects >
		D3D12_COMMAND_QUEUE_DESC commandQueueDescriptor = {};
		commandQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDescriptor.NodeMask = 0;
		commandQueueDescriptor.Priority = 0;
		commandQueueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HR(mDevice->CreateCommandQueue(&commandQueueDescriptor, IID_PPV_ARGS(mCommandQueue.GetAddressOf())));
		HR(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocator.GetAddressOf())));
		HR(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(mGraphicsCommandList.GetAddressOf())));
		mGraphicsCommandList->Close();

		// < SwapChain >
		DXGI_SWAP_CHAIN_DESC swapChainDescriptor = {};
		swapChainDescriptor.BufferCount = gBackBufferCount;
		swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDescriptor.OutputWindow = mHwnd;
		swapChainDescriptor.SampleDesc.Count = 1;
		swapChainDescriptor.SampleDesc.Quality = 0;
		swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDescriptor.Windowed = true;
		swapChainDescriptor.BufferDesc.Format = gBackBufferFormat;
		swapChainDescriptor.BufferDesc.Width = mClientWidth;
		swapChainDescriptor.BufferDesc.Height = mClientHeight;
		swapChainDescriptor.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDescriptor.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		HR(mFactory4->CreateSwapChain(mCommandQueue.Get(), &swapChainDescriptor, mSwapChain.GetAddressOf()));

		// < Fence >
		HR(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetAddressOf())));

		// < Descriptors >
		mRtvSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		mDsvSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		mCbvSrvUavSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptor = {};
		rtvDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvDescriptor.NodeMask = 0;
		rtvDescriptor.NumDescriptors = gBackBufferCount;
		rtvDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		HR(mDevice->CreateDescriptorHeap(&rtvDescriptor, IID_PPV_ARGS(mRtvDescriptorHeap.GetAddressOf())));

		D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptor = {};
		dsvDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvDescriptor.NodeMask = 0;
		dsvDescriptor.NumDescriptors = 1;
		dsvDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		HR(mDevice->CreateDescriptorHeap(&rtvDescriptor, IID_PPV_ARGS(mDsvDescriptorHeap.GetAddressOf())));

		// < OnResize >
		OnResize(screenWidth, screenHeight);
	}

	void Renderer::OnResize(UINT screenWidth, UINT screenHeight)
	{
		assert(mDevice);
		assert(mSwapChain);
		assert(mCommandAllocator);

		FlushCommandList();

		HR(mGraphicsCommandList->Reset(mCommandAllocator.Get(), nullptr));
		for (UINT i = 0; i < gBackBufferCount; ++i)
		{
			mBackBuffer[i].Reset();
		}
		mDepthStencilBuffer.Reset();

		mCurrBackBufferIndex = 0;
		HR(mSwapChain->ResizeBuffers(gBackBufferCount, screenWidth, screenHeight,
			gBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < gBackBufferCount; ++i)
		{
			HR(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mBackBuffer[i].GetAddressOf())));
			mDevice->CreateRenderTargetView(mBackBuffer[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, mRtvSize);
		}

		D3D12_RESOURCE_DESC dsvResourceDescriptor = {};
		dsvResourceDescriptor.Alignment = 0;
		dsvResourceDescriptor.DepthOrArraySize = 1;
		dsvResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsvResourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		dsvResourceDescriptor.Format = gDepthStencilFormat;
		dsvResourceDescriptor.Width = mClientWidth;
		dsvResourceDescriptor.Height = mClientHeight;
		dsvResourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsvResourceDescriptor.MipLevels = 1;
		dsvResourceDescriptor.SampleDesc.Count = 1;
		dsvResourceDescriptor.SampleDesc.Quality = 0;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = gDepthStencilFormat;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		HR(mDevice->CreateCommittedResource(unmove(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)), D3D12_HEAP_FLAG_NONE,
			&dsvResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));


		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescriptor = {};
		depthStencilViewDescriptor.Flags = D3D12_DSV_FLAG_NONE;
		depthStencilViewDescriptor.Format = gDepthStencilFormat;
		depthStencilViewDescriptor.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDescriptor.Texture2D.MipSlice = 0;

		mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(),
			&depthStencilViewDescriptor, mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		mGraphicsCommandList->ResourceBarrier(1,
			unmove(&CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE)));

		mGraphicsCommandList->Close();
		ID3D12CommandList* cmdLists[] = { mGraphicsCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		FlushCommandList();

		mScissorRect = { 0, 0, (LONG)screenWidth, (LONG)screenHeight };
		mViewPort = { 0.0f, 0.0f, (FLOAT)screenWidth, (FLOAT)screenHeight, 0.0f, 1.0f };
	}

	void Renderer::FlushCommandList()
	{
		++mCurrFenceCount;
		HR(mCommandQueue->Signal(mFence.Get(), mCurrFenceCount));

		if (mFence->GetCompletedValue() < mCurrFenceCount)
		{
			HANDLE hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
			HR(mFence->SetEventOnCompletion(mCurrFenceCount, hEvent));
			WaitForSingleObject(hEvent, 0xffffffff);
			CloseHandle(hEvent);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Renderer::rtvHandle()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrBackBufferIndex, mRtvSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Renderer::dsvHandle()
	{
		return mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	}
}

LRESULT __stdcall WindowProcedure(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam)
{
	return orangelie::Renderer::gGameApp->MessageHandler(hWnd, hMessage, wParam, lParam);
}