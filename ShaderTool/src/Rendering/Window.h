#pragma once

#include <Windows.h>

class Window
{
public:
	Window() = default;
	~Window() = default;

	bool Init(HINSTANCE hInstance, WNDPROC wndProc);

private:
	HWND _Handler;
};