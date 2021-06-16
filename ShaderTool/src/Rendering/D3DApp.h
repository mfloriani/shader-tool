#pragma once

#include <memory>

#include "Window.h"
#include "Renderer.h"

class D3DApp
{
public:
	static D3DApp& Get()
	{
		static D3DApp instance;
		return instance;
	}

	~D3DApp();
	D3DApp(const D3DApp& other) = delete;
	void operator=(const D3DApp& other) = delete;

	bool Init(HINSTANCE hInstance);
	void Run();
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnUpdate();
	void OnRender();

	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);

	void OnResize(int width, int height);

private:
	D3DApp() = default;

private:
	std::unique_ptr<Window>   _Window;
	std::unique_ptr<Renderer> _Renderer;
};