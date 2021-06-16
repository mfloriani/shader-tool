#include "pch.h"
#include "D3DApp.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

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
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			OnUpdate();
			OnRender();
		}
	}
	catch (DxException& e)
	{
		LOG_CRITICAL(L"DX Error: {0}", e.ToString().c_str());
	}
}

LRESULT D3DApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

		OnResize(width, height);
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
	//LOG_WARN("OnRender()");
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	_Renderer->Clear();
	_Renderer->NewUIFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	_Renderer->RenderUI();
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

void D3DApp::OnResize(int width, int height)
{
	if (_Renderer->HasToResizeBuffer(width, height))
	{
		_Renderer->Flush();
		ImGui_ImplDX12_InvalidateDeviceObjects();
		_Renderer->OnResize(width, height);
		ImGui_ImplDX12_CreateDeviceObjects();
	}
}
