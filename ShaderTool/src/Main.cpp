#include "pch.h"
#include "Rendering\D3DApp.h"

int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // Enable run-time memory check for debug builds.
#endif

	return WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
	if (D3DApp::Get().Init(hInstance))
	{
		D3DApp::Get().Run();
	}

	return 0;
}



