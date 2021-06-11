#include "pch.h"
#include "D3DApp.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return D3DApp::Get().WndProc(hWnd, uMsg, wParam, lParam);
}

D3DApp::~D3DApp()
{

}

bool D3DApp::Init(HINSTANCE hInstance)
{
	Log::Init();

	_Window = std::make_unique<Window>();
	if (!_Window->Init(hInstance, ::WndProc))
		return false;

	_Renderer = std::make_unique<Renderer>(_Window.get());
	if (!_Renderer->Init())
		return false;

	return true;
}

void D3DApp::Run()
{
	try
	{
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
				
			}
		}
	}
	catch (DxException& e)
	{
		LOG_CRITICAL(L"D3D Error: {0}", e.ToString().c_str());
	}
}

LRESULT D3DApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		OnUpdate();
		OnRender();
		break;
	}
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

		break;
	}
	// The default window procedure will play a system notification sound 
	// when pressing the Alt+Enter keyboard combination if this message is 
	// not handled.
	case WM_SYSCHAR:
		break;
	case WM_SIZE:
	{
		RECT clientRect = {};
		::GetClientRect(_Window->GetHandler(), &clientRect);

		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;

		_Renderer->OnResize(width, height);
		break;
	}
	//case WM_EXITSIZEMOVE:
	//	break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void D3DApp::OnUpdate()
{
}

void D3DApp::OnRender()
{
	_Renderer->Clear();



	_Renderer->Present();
}

void D3DApp::OnKeyDown(WPARAM key)
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

void D3DApp::OnKeyUp(WPARAM key)
{

}
