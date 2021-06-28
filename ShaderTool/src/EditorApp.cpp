#include "pch.h"
#include "EditorApp.h"
#include "Editor\NodeGraphEditor.h"

using namespace DirectX;
using namespace D3DUtil;

using Microsoft::WRL::ComPtr;

EditorApp::EditorApp(HINSTANCE hInstance) : D3DApp(hInstance)
{

}

EditorApp::~EditorApp()
{
	color_editor.Quit();
}
	

bool EditorApp::Init()
{	
	if (!D3DApp::Init())
		return false;

	BuildRootSignature();
	BuildShadersAndInputLayout();
	LoadDefaultMeshes();

	color_editor.Init();

	return true;
}

void EditorApp::Run()
{
	Log::Init();

	if (!Init())
		return;
	
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
	catch (D3DUtil::DxException& e)
	{
		LOG_CRITICAL(L"DX Error: {0}", e.ToString().c_str());
	}
}



void EditorApp::OnUpdate()
{
	_Timer.Tick();
	SwapFrameResource(); // cycle through the circular frame resource array

}

void EditorApp::OnRender()
{
	// CLEAR
	auto commandAllocator = _CurrFrameResource->CmdListAlloc;
	auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];

	commandAllocator->Reset();
	_CommandList->Reset(commandAllocator.Get(), nullptr);

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	_CommandList->RSSetViewports(1, &_ScreenViewport);
	_CommandList->RSSetScissorRects(1, &_ScissorRect);

	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		_CommandList->ResourceBarrier(1, &barrier);
	}

	FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
		_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		_CurrentBackBufferIndex,
		_RTVDescriptorSize
	);

	auto& dsv = _DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	_CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	_CommandList->ClearDepthStencilView(
		dsv,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0, 0,
		nullptr
	);

	// Specify the buffers we are going to render to.
	_CommandList->OMSetRenderTargets(1, &rtv, true, &dsv);
	
	NewUIFrame();

	RenderUIDockSpace();
	color_editor.show();

	RenderUI();

	// PRESENT
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		_CommandList->ResourceBarrier(1, &barrier);
	}

	ThrowIfFailed(_CommandList->Close());

	ID3D12CommandList* const commandLists[] = { _CommandList.Get() };
	_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	UINT syncInterval = _VSync ? 1 : 0;
	UINT presentFlags = _TearingSupport && !_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	ThrowIfFailed(_SwapChain->Present(syncInterval, presentFlags));

	_CurrentBackBufferIndex = (_CurrentBackBufferIndex + 1) % NUM_BACK_BUFFERS;

	// Advance the fence value to mark commands up to this fence point.
	_CurrFrameResource->FenceValue = ++_CurrentFenceValue;

	// Add an instruction to the command queue to set a new fence point.
	// Because we are on the GPU timeline, the new fence point won't be
	// set until the GPU finishes processing all the commands prior to this Signal().
	_CommandQueue->Signal(_Fence.Get(), _CurrentFenceValue);

}

void EditorApp::SwapFrameResource()
{
	// cycle through the frames to continue writing commands to avoid having the GPU idle
	_CurrFrameResourceIndex = (_CurrFrameResourceIndex + 1) % NUM_FRAMES;
	_CurrFrameResource = _FrameResources[_CurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (_CurrFrameResource->FenceValue != 0 && _Fence->GetCompletedValue() < _CurrFrameResource->FenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(_Fence->SetEventOnCompletion(_CurrFrameResource->FenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void EditorApp::OnKeyDown(WPARAM key)
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

void EditorApp::OnKeyUp(WPARAM key)
{

}


void EditorApp::BuildRootSignature()
{
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER rootParameters[2] = {};

	// Create root CBV.
	rootParameters[0].InitAsConstantBufferView(0);
	rootParameters[1].InitAsConstantBufferView(1);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		2, 
		rootParameters, 
		0, 
		nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc, 
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), 
		errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		LOG_ERROR("Error at D3D12SerializeRootSignature()");
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(
		_Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(_RootSignature.GetAddressOf()))
	);
}

void EditorApp::BuildShadersAndInputLayout()
{
	ComPtr<ID3DBlob> vsBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"DefaultVS.cso", &vsBlob));
	_Shaders.insert(std::pair("default_vs", vsBlob));

	ComPtr<ID3DBlob> psBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"DefaultPS.cso", &psBlob));
	_Shaders.insert(std::pair("default_ps", psBlob));

	_InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void EditorApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC defaultPSO;
	ZeroMemory(&defaultPSO, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	defaultPSO.InputLayout = { _InputLayout.data(), (UINT)_InputLayout.size() };
	defaultPSO.pRootSignature = _RootSignature.Get();
	defaultPSO.VS =
	{
		reinterpret_cast<BYTE*>(_Shaders["default_vs"]->GetBufferPointer()),
		_Shaders["default_vs"]->GetBufferSize()
	};
	defaultPSO.PS =
	{
		reinterpret_cast<BYTE*>(_Shaders["default_ps"]->GetBufferPointer()),
		_Shaders["default_ps"]->GetBufferSize()
	};
	defaultPSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	defaultPSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	defaultPSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	defaultPSO.SampleMask = UINT_MAX;
	defaultPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	defaultPSO.NumRenderTargets = 1;
	defaultPSO.RTVFormats[0] = _BackBufferFormat;
	defaultPSO.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	defaultPSO.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
	defaultPSO.DSVFormat = _DepthStencilFormat;
	
	ThrowIfFailed(_Device->CreateGraphicsPipelineState(&defaultPSO, IID_PPV_ARGS(&_PSOs["default"])));
}

void EditorApp::LoadDefaultMeshes()
{
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f)}),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f)}),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f)}),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f)}),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f)}),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f)}),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f)}),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f)}),
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	
	mBoxGeo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(
		_Device.Get(),
		_CommandList.Get(), 
		vertices.data(), 
		vbByteSize, 
		mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(
		_Device.Get(),
		_CommandList.Get(), 
		indices.data(), 
		ibByteSize, 
		mBoxGeo->IndexBufferUploader);

	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;

	_Meshes.insert(std::make_pair("primitives", std::move(mBoxGeo)));
}

void EditorApp::RenderUIDockSpace()
{
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
}