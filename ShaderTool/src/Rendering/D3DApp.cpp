#include "pch.h"
#include "D3DApp.h"
#include "Editor\NodeGraphEditor.h"


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

	_Window->SetFullscreen(true);

	color_editor.Init();

	return true;
}

void D3DApp::Run()
{
	_Timer.Reset();
	try
	{
		MSG msg = { 0 };
		while (_IsRunning)
		{
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
					_IsRunning = false;
			}

			if (!_IsRunning)
				break;

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
		LOG_TRACE("WM_CLOSE");
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		LOG_TRACE("WM_DESTROY");
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
	_Timer.Tick();
}

void D3DApp::OnRender()
{
	//LOG_WARN("OnRender()");
	
	_Renderer->Clear();
	_Renderer->NewUIFrame();

	static bool docking_open = true;
	static bool docking_opt_fullscreen = true;
	static bool docking_opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // no padding
	ImGui::Begin("DockSpace", &docking_open, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2); // fullscreen related

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	else
	{
		LOG_ERROR("ImGui Docking is disabled");
	}

	//if (ImGui::BeginMenuBar())
	//{
	//	if (ImGui::BeginMenu("Options"))
	//	{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &docking_opt_fullscreen);
			//ImGui::MenuItem("Padding", NULL, &docking_opt_padding);
			//ImGui::Separator();

			//if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
			//if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
			//if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
			//if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
			//if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, docking_opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
			//ImGui::Separator();

			//if (ImGui::MenuItem("Close", NULL, false, docking_open != NULL))
			//	docking_open = false;
	//		ImGui::EndMenu();
	//	}
	//	ImGui::EndMenuBar();
	//}


	ImGui::End();

	//static bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);
	
	color_editor.show();

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
