#include "pch.h"
#include "ShaderToolApp.h"

using namespace DirectX;
using namespace D3DUtil;

using Microsoft::WRL::ComPtr;

ShaderToolApp::ShaderToolApp(HINSTANCE hInstance) : D3DApp(hInstance), _RootNodeId(-1)
{

}

ShaderToolApp::~ShaderToolApp()
{
	LOG_TRACE("ShaderToolApp::~ShaderToolApp()");
	FlushCommandQueue();
}

void ShaderToolApp::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case VK_ESCAPE:
	{
		LOG_TRACE("ESC pressed");
		//PostQuitMessage(0);
		break;
	}
	default:
		break;
	}
}

void ShaderToolApp::OnKeyUp(WPARAM key)
{

}

void ShaderToolApp::Run()
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

void ShaderToolApp::OnResize(uint32_t width, uint32_t height)
{
	D3DApp::OnResize(width, height);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&_Proj, P);

	// TODO: move this to the resize event of the render target node 
	//if(_RenderTarget)
	//	_RenderTarget->OnResize(width, height);
}

void ShaderToolApp::UpdateCamera()
{
	// Convert Spherical to Cartesian coordinates.
	_EyePos.x = _Radius * sinf(_Phi) * cosf(_Theta);
	_EyePos.z = _Radius * sinf(_Phi) * sinf(_Theta);
	_EyePos.y = _Radius * cosf(_Phi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(_EyePos.x, _EyePos.y, _EyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&_View, view);
}

void ShaderToolApp::UpdatePerFrameCB()
{
	XMMATRIX view = XMLoadFloat4x4(&_View);
	XMMATRIX proj = XMLoadFloat4x4(&_Proj);
	XMMATRIX rtProj = XMLoadFloat4x4(&_RTProj);

	//XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	//XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	//XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	//XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&_FrameCB.View, XMMatrixTranspose(view));
	//XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&_FrameCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&_FrameCB.RTProj, XMMatrixTranspose(rtProj));
	//XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	//XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	//XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	//mMainPassCB.EyePosW = mEyePos;
	//mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	//mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	//mMainPassCB.NearZ = 1.0f;
	//mMainPassCB.FarZ = 1000.0f;
	//mMainPassCB.TotalTime = gt.TotalTime();
	//mMainPassCB.DeltaTime = gt.DeltaTime();

	auto currPassCB = _CurrFrameResource->FrameCB.get();
	currPassCB->CopyData(0, _FrameCB);
}

void ShaderToolApp::UpdatePerObjectCB()
{
	auto currObjectCB = _CurrFrameResource->ObjectCB.get();
	XMMATRIX world = XMMatrixIdentity();

	// BOX ROTATION
	{
		_Entity.Rotation = { 0.f, _Timer.TotalTime(), 0.f };
		auto rotation = XMMatrixRotationY(_Entity.Rotation.y);
		auto scale = XMMatrixScaling(_Entity.Scale.x, _Entity.Scale.y, _Entity.Scale.z);
		world = XMMatrixMultiply(scale, rotation);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		
		objConstants.Color = _Entity.Color;

		int objCBIndex = _Entity.Id; // TODO: handle multiple objects
		currObjectCB->CopyData(objCBIndex, objConstants);
	}

#if 0
	// QUAD
	{
		//world = XMMatrixIdentity();
		FLOAT size = 10.f;
		world = XMMatrixScaling(1.f * size, 1.f * size, 1.f * size);
	
		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		int objCBIndex = 1;
		currObjectCB->CopyData(objCBIndex, objConstants);
	}
#endif
}

void ShaderToolApp::OnUpdate()
{
	_Timer.Tick();
	UpdateCamera();
	SwapFrameResource();
	UpdatePerObjectCB();
	UpdatePerFrameCB();
}

void ShaderToolApp::OnRender()
{
	// CLEAR
	auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];
	auto commandAllocator = _CurrFrameResource->CmdListAlloc;
	commandAllocator->Reset();
	
	_CommandList->Reset(commandAllocator.Get(), nullptr);
	_CommandList->SetGraphicsRootSignature(_RootSignature.Get());

	auto frameCB = _CurrFrameResource->FrameCB->Resource();
	_CommandList->SetGraphicsRootConstantBufferView(1, frameCB->GetGPUVirtualAddress());
	
	_CurrFrameResource->RenderTargetPSO = _RenderTargetPSO;

	EvaluateGraph();
	
	// Render to back buffer
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		_CommandList->ResourceBarrier(1, &barrier);
	}

	FLOAT clearColor[] = { 0.7f, 0.7f, 1.0f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
		_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		_CurrentBackBufferIndex,
		_RtvDescriptorSize
	);

	auto dsv = _DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	_CommandList->RSSetViewports(1, &_ScreenViewport);
	_CommandList->RSSetScissorRects(1, &_ScissorRect);
	_CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	_CommandList->ClearDepthStencilView(
		dsv,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0, 0,
		nullptr
	);

	_CommandList->OMSetRenderTargets(1, &rtv, true, &dsv);
	//_CommandList->SetGraphicsRootSignature(_RootSignature.Get());
	_CommandList->SetPipelineState(_BackBufferPSO->GetPSO());

	NewUIFrame();
	RenderUIDockSpace();
	UpdateNodeGraph();
	RenderNodeGraph();
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

// cycle through frames to continue writing commands avoiding the GPU getting idle
void ShaderToolApp::SwapFrameResource()
{
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



void ShaderToolApp::RenderUIDockSpace()
{
	static bool docking_open = true;
	static bool docking_opt_fullscreen = true;
	static bool docking_opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags overlay_window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	overlay_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	overlay_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // no padding
	ImGui::Begin("DockSpace", &docking_open, overlay_window_flags);
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

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("New", NULL, false, docking_open != NULL)) LOG_TRACE("New");
			if (ImGui::MenuItem("Save", NULL, false, docking_open != NULL)) LOG_TRACE("Save");
			if (ImGui::MenuItem("Load", NULL, false, docking_open != NULL)) LOG_TRACE("Load");
			if (ImGui::MenuItem("Close", NULL, false, docking_open != NULL)) LOG_TRACE("Close");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::End();
}

