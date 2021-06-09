#include "pch.h"
#include "Rendering\Window.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void OnKeyDown(WPARAM key);
void OnKeyUp(WPARAM key);

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
	Log::Init();

	Window window;
	if (!window.Init(hInstance, WndProc))
		return 1;

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			
			//mTimer.Tick();

			//CalculateFrameStats();
			//Update(mTimer);
			//Draw(mTimer);
			
		}
	}

	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, 0));
	//if (!pWnd)
	//	return DefWindowProc(hWnd, uMsg, wParam, lParam);

	//if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
	//	return true;

	switch (uMsg)
	{
	case WM_CLOSE:
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		OnKeyDown(wParam);
		break;

	case WM_KEYUP:
		OnKeyUp(wParam);
		break;

	case WM_MOUSEMOVE:
	{
		//auto mousePos = MOUSEMOVEPOINT();
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		//LOG_TRACE("WM_MOUSEMOVE {0},{1}", xPos, yPos);

	}
	break;

	//case WM_SIZE:
		//_renderer->CreateWindowSizeResources();
		//break;

	case WM_EXITSIZEMOVE:
		//_renderer->OnResize();
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case VK_ESCAPE:
	{
		LOG_TRACE("ESC pressed");
		PostQuitMessage(0);
		break;
	}
	default:
		break;
	}
}

void OnKeyUp(WPARAM key)
{

}
