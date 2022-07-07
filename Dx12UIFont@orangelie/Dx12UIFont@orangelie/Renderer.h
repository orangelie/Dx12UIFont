#pragma once

#include "Utils.h"

namespace orangelie
{
	class Renderer
	{
	public:
		static Renderer* gGameApp;

		CLASSIFICATION_H(Renderer);

		void Initialize(UINT screenWidth, UINT screenHeight);
		void Render();

	protected:
		virtual void update(float dt) = 0;
		virtual void draw(float dt) = 0;

	private:
		void BuildWindows(UINT screenWidth, UINT screenHeight);

	private:
		LPCSTR mWndClassName = "orangelieApp";
		HWND mHwnd;
		HINSTANCE mModuleHandle;
		UINT mFullscreenWidth, mFullscreenHeight, mClientWidth, mClientHeight;

	};
}

LRESULT __stdcall WindowProcedure(HWND hWnd, UINT hMessage, WPARAM wParam, LPARAM lParam);