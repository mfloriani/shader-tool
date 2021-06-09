#include "pch.h"
#include "Window.h"
#include "Constants.h"

bool Window::Init(HINSTANCE hInstance, WNDPROC wndProc)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(Window); // used for the pointer of this object passed to the static callback
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	RECT wr = { 0, 0, ScreenWidth, ScreenHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	int centerScreenX = static_cast<int>((GetSystemMetrics(SM_CXSCREEN) - ScreenWidth) * 0.5);
	int centerScreenY = static_cast<int>((GetSystemMetrics(SM_CYSCREEN) - ScreenHeight) * 0.5);

	_Handler = CreateWindowEx(NULL,
		L"WindowClass",
		L"ShaderTool",
		WS_OVERLAPPEDWINDOW,
		centerScreenX,
		centerScreenY,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (_Handler == NULL)
	{
		LOG_ERROR("Error creating window");
		return false;
	}

	// passing `this` pointer to the window, so I can call member function in wnd callback
	SetWindowLongPtrW(_Handler, 0, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(_Handler, SW_SHOW);
	UpdateWindow(_Handler);

    return true;
}
