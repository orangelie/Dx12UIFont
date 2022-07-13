#pragma once

#include "Utils.h"

namespace orangelie
{
	class Renderer
	{
	public:
		static Renderer* gGameApp;

		CLASSIFICATION_H(Renderer);
		static LRESULT MessageHandler(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam);

		void Initialize(UINT screenWidth, UINT screenHeight);
		void Render();

	protected:
		virtual void update(float dt) = 0;
		virtual void draw(float dt) = 0;

	private:
		void BuildWindows(UINT screenWidth, UINT screenHeight);
		void BuildDxgiAndD3D12(UINT screenWidth, UINT screenHeight);

	private:
		LPCSTR mWndClassName = "orangelieApp";
		HWND mHwnd;
		HINSTANCE mModuleHandle;
		UINT mFullscreenWidth, mFullscreenHeight, mClientWidth, mClientHeight;

		
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice = nullptr;

	};
}

LRESULT __stdcall WindowProcedure(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam);