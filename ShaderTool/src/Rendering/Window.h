#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class Window
{
public:
	Window() = default;
	~Window() = default;

	bool Init(HINSTANCE hInstance, WNDPROC wndProc);
	HWND GetHandler() const { return _Handler; }
	
	std::pair<uint32_t, uint32_t> GetSize() const;
	void SetFullscreen(bool fullscreen);

private:
	HWND _Handler{ nullptr };
	RECT _WindowedRect{};
	bool _Fullscreen{ false };
};