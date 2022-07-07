#include "Dx12UIFont.h"

int __stdcall WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ INT hCmdShow
)
{
	try
	{
		std::unique_ptr<orangelie::Renderer> appRenderer(new orangelie::Dx12UIFont);

		appRenderer->Initialize(1080, 860);
		appRenderer->Render();

	}
	catch (const std::exception& e)
	{
		MessageBoxA(0, e.what(), "< Exception >", MB_ICONWARNING);
		return -1;
	}

	return 0;
}