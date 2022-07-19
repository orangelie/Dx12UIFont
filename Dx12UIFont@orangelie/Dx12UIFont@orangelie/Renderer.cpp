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
				mGameTimer.Stop();
			}
			else {
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
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mIsMinimized = false;
				mIsMaximized = true;
				// Resize;
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (mIsMinimized)
				{
					mIsMinimized = false;
					mIsMaximized = false;
					// Resize;
				}
				else if(mIsMaximized)
				{
					mIsMinimized = false;
					mIsMaximized = false;
					// Resize;
				}
				else if (mIsResizing)
				{

				}
			}

			return 0;

		case WM_ENTERSIZEMOVE:
			mGameTimer.Stop();
			mIsResizing = true;
			return 0;

		case WM_EXITSIZEMOVE:
			mGameTimer.Start();
			mIsResizing = false;
			return 0;

		case WM_DESTROY: case WM_CLOSE:
			PostQuitMessage(0); return 0;
		}

		return DefWindowProc(hWnd, hMessage, wParam, lParam);
	}

	void Renderer::Initialize(UINT screenWidth, UINT screenHeight)
	{
		BuildWindows(screenWidth, screenHeight);
		BuildDxgiAndD3D12(screenWidth, screenHeight);
	}

	void Renderer::Render()
	{
		MSG msg = {};
		mGameTimer.Reset();

		for (;;)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				DispatchMessage(&msg);
				TranslateMessage(&msg);
			}

			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				mGameTimer.Tick();

				update(mGameTimer.DeltaTime());
				draw(mGameTimer.DeltaTime());
			}
		}
	}

	void Renderer::BuildWindows(UINT screenWidth, UINT screenHeight)
	{
		mModuleHandle = GetModuleHandle(nullptr);

		mClientWidth = screenWidth;
		mClientHeight = screenHeight;
		mFullscreenWidth = GetSystemMetrics(SM_CXSCREEN);
		mFullscreenHeight = GetSystemMetrics(SM_CYSCREEN);

		WNDCLASSEX wndClassEx = {};
		wndClassEx.cbClsExtra = 0;
		wndClassEx.cbSize = sizeof(WNDCLASSEX);
		wndClassEx.cbWndExtra = 0;
		wndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClassEx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClassEx.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wndClassEx.hIconSm = wndClassEx.hIcon;
		wndClassEx.hInstance = mModuleHandle;
		wndClassEx.lpszClassName = mWndClassName;
		wndClassEx.lpfnWndProc = WindowProcedure;
		wndClassEx.lpszMenuName = nullptr;
		wndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		RegisterClassEx(&wndClassEx);

		DEVMODE devMode = {};
		devMode.dmBitsPerPel = 32;
		devMode.dmPelsWidth = screenWidth;
		devMode.dmPelsHeight = screenHeight;
		devMode.dmFields = DM_BITSPERPEL | DM_PELSHEIGHT | DM_PELSWIDTH;

		ChangeDisplaySettings(&devMode, 0);

		mHwnd = CreateWindowEx(
			WS_EX_OVERLAPPEDWINDOW,
			mWndClassName,
			mWndClassName,
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW,
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
		// < Device >
		HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(mDevice.GetAddressOf())));

		//mDevice->CreateCommandList()
	}
}

LRESULT __stdcall WindowProcedure(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam)
{
	return orangelie::Renderer::gGameApp->MessageHandler(hWnd, hMessage, wParam, lParam);
}