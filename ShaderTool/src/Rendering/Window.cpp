#include "pch.h"
#include "Window.h"
#include "Defines.h"

bool Window::Init(HINSTANCE hInstance, WNDPROC wndProc)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	RECT wr = { 0, 0, static_cast<LONG>(INIT_CLIENT_WIDTH), static_cast<LONG>(INIT_CLIENT_HEIGHT) };
	::AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = wr.right - wr.left;
	int windowHeight = wr.bottom - wr.top;

	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	// Center the window within the screen. Clamp to 0, 0 for the top-left corner.
	int windowX = (std::max<int>(0, static_cast<int>((screenWidth - windowWidth) * 0.5f)));
	int windowY = (std::max<int>(0, static_cast<int>((screenHeight - windowHeight) * 0.5f)));

	_Handler = CreateWindowEx(NULL,
		L"WindowClass",
		L"ShaderTool",
		WS_OVERLAPPEDWINDOW,
		windowX,
		windowY,
		windowWidth,
		windowHeight,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (_Handler == NULL)
	{
		LOG_CRITICAL("Error creating window");
		return false;
	}
	
	::GetWindowRect(_Handler, &_WindowedRect);

    return true;
}

std::pair<uint32_t, uint32_t> Window::GetSize() const
{
	RECT clientRect = {};
	::GetClientRect(_Handler, &clientRect);

	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	return { width, height };
}

void Window::SetFullscreen(bool fullscreen)
{
	if (_Fullscreen != fullscreen)
	{
		_Fullscreen = fullscreen;

		if (_Fullscreen) // Switching to fullscreen.
		{
			// Store the current window dimensions so they can be restored 
			// when switching out of fullscreen state.
			::GetWindowRect(_Handler, &_WindowedRect);

			// Set the window style to a borderless window so the client area fills
			// the entire screen.
			UINT windowStyle = WS_OVERLAPPEDWINDOW;// &~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
			::SetWindowLongW(_Handler, GWL_STYLE, windowStyle);

			// Query the name of the nearest display device for the window.
			// This is required to set the fullscreen dimensions of the window
			// when using a multi-monitor setup.
			HMONITOR hMonitor = ::MonitorFromWindow(_Handler, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorInfo);

			::SetWindowPos(_Handler, HWND_TOPMOST,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(_Handler, SW_MAXIMIZE);
		}
		else
		{
			// Restore all the window decorators.
			::SetWindowLong(_Handler, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			::SetWindowPos(_Handler, HWND_NOTOPMOST,
				_WindowedRect.left,
				_WindowedRect.top,
				_WindowedRect.right - _WindowedRect.left,
				_WindowedRect.bottom - _WindowedRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(_Handler, SW_NORMAL);
		}
	}
}