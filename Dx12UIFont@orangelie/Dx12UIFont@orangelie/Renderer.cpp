#include "Renderer.h"

namespace orangelie
{
	Renderer* Renderer::gGameApp = nullptr;

	CLASSIFICATION_C2(Renderer,
		{
			gGameApp = this;
		});

	void Renderer::Initialize(UINT screenWidth, UINT screenHeight)
	{
		BuildWindows(screenWidth, screenHeight);
	}

	void Renderer::Render()
	{
		MSG msg = {};

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

			

			update(0.1f);
			draw(0.1f);
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
}

LRESULT __stdcall WindowProcedure(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam)
{
	switch (hMessage)
	{
	case WM_DESTROY: case WM_CLOSE:
		PostQuitMessage(0); return 0;
	}

	return DefWindowProc(hWnd, hMessage, wParam, lParam);
}