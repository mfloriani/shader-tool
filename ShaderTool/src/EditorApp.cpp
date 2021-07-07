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
	LOG_TRACE("EditorApp::~EditorApp()");
	FlushCommandQueue();
	color_editor.Quit();
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

bool EditorApp::Init()
{
	if (!D3DApp::Init())
		return false;
	
	//OnResize(_CurrentBufferWidth, _CurrentBufferHeight);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	UINT width = 1024;
	UINT height = 1024;
	auto aspectRatio = static_cast<float>(width) / height;
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio, 1.0f, 1000.0f);
	XMStoreFloat4x4(&_RTProj, P);

	// RENDER-TO-TEXTURE
	_RenderTexture = std::make_unique<RenderTexture>(
		_Device.Get(),
		_BackBufferFormat,
		width,
		height
	);

	auto commandAllocator = _CurrFrameResource->CmdListAlloc;
	_CommandList->Reset(commandAllocator.Get(), nullptr);

	CreateDescriptorHeaps();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();
	LoadDefaultMeshes();

	ThrowIfFailed(_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { _CommandList.Get() };
	_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	color_editor.Init();

	return true;
}

void EditorApp::CreateDescriptorHeaps()
{
	// SRV descriptor heap for render-to-texture
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_RenderTexSrvDescriptorHeap)));

	_RenderTexture->CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_RenderTexSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, _CbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_RenderTexSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, _CbvSrvUavDescriptorSize),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), NUM_BACK_BUFFERS, _RtvDescriptorSize)
	);

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

void EditorApp::OnResize(uint32_t width, uint32_t height)
{
	D3DApp::OnResize(width, height);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&_Proj, P);

	// TODO: move this to the resize event of the render target node 
	if(_RenderTexture)
		_RenderTexture->OnResize(width, height);
}

void EditorApp::UpdateCamera()
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

void EditorApp::UpdatePerFrameCB()
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

void EditorApp::UpdatePerObjectCB()
{
	auto currObjectCB = _CurrFrameResource->ObjectCB.get();
	XMMATRIX world = XMMatrixIdentity();

	// BOX ROTATION
	{
		FLOAT size = 10.f;
		auto scale = XMMatrixScaling(1.f * size, 1.f * size, 1.f * size);
		auto rotation = XMMatrixRotationY(_Timer.TotalTime());

		world = XMMatrixMultiply(scale, rotation);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		int objCBIndex = 0; // TODO: handle multiple objects
		currObjectCB->CopyData(objCBIndex, objConstants);
	}

	// QUAD
	{
		//world = XMMatrixIdentity();
		FLOAT size = 10.f;
		world = XMMatrixScaling(1.f * size, 1.f * size, 1.f * size);
	
		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		int objCBIndex = 1; // TODO: handle multiple objects
		currObjectCB->CopyData(objCBIndex, objConstants);
	}

}

void EditorApp::OnUpdate()
{
	_Timer.Tick();
	UpdateCamera();
	SwapFrameResource();
	UpdatePerObjectCB();
	UpdatePerFrameCB();
}

void EditorApp::OnRender()
{
	// CLEAR
	auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];
	auto commandAllocator = _CurrFrameResource->CmdListAlloc;
	commandAllocator->Reset();
	
	_CommandList->Reset(commandAllocator.Get(), _PSOs["render_target"].Get());
	_CommandList->SetGraphicsRootSignature(_RootSignature.Get());

	auto frameCB = _CurrFrameResource->FrameCB->Resource();
	_CommandList->SetGraphicsRootConstantBufferView(1, frameCB->GetGPUVirtualAddress());

	//////////////////////////// 
	// 
	// RENDER TO TEXTURE
	
	_CommandList->ResourceBarrier(
		1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(
			_RenderTexture->GetResource(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	_CommandList->RSSetViewports(1, &_RenderTexture->GetViewPort());
	_CommandList->RSSetScissorRects(1, &_RenderTexture->GetScissorRect());
	_CommandList->ClearRenderTargetView(_RenderTexture->RTV(), _RenderTexture->GetClearColor(), 0, nullptr);
	_CommandList->OMSetRenderTargets(1, &_RenderTexture->RTV(), true, nullptr);
	
	// BOX
	{
		auto& box = _Meshes["box_geo"]; // TODO: avoid direct access

		_CommandList->IASetVertexBuffers(0, 1, &box->VertexBufferView());
		_CommandList->IASetIndexBuffer(&box->IndexBufferView());
		_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		auto objectCB = _CurrFrameResource->ObjectCB->Resource();

		UINT objCBIndex = 0; // TODO: check this
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
		objCBAddress += objCBIndex * objCBByteSize;

		_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		_CommandList->DrawIndexedInstanced(
			box->DrawArgs["box"].IndexCount,
			1,
			box->DrawArgs["box"].StartIndexLocation,
			box->DrawArgs["box"].BaseVertexLocation,
			0);
	}

	_CommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			_RenderTexture->GetResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ));

	//////////////////////////

	
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
	_CommandList->SetPipelineState(_PSOs["back_buffer"].Get());

	//CD3DX12_GPU_DESCRIPTOR_HANDLE tex(_RenderTexSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	//tex.Offset(0, _CbvSrvUavDescriptorSize);
	ID3D12DescriptorHeap* const descHeapList[] = { _RenderTexSrvDescriptorHeap.Get() };
	_CommandList->SetDescriptorHeaps(_countof(descHeapList), descHeapList);
	_CommandList->SetGraphicsRootDescriptorTable(2, _RenderTexture->SRV());
	
	// QUAD
	{
		auto& quad = _Meshes["quad_geo"]; // TODO: avoid direct access

		_CommandList->IASetVertexBuffers(0, 1, &quad->VertexBufferView());
		_CommandList->IASetIndexBuffer(&quad->IndexBufferView());
		_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		auto objectCB = _CurrFrameResource->ObjectCB->Resource();

		UINT objCBIndex = 1; // TODO: this should not be hardcoded
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
		objCBAddress += objCBIndex * objCBByteSize;

		_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		_CommandList->DrawIndexedInstanced(
			quad->DrawArgs["quad"].IndexCount,
			1,
			quad->DrawArgs["quad"].StartIndexLocation,
			quad->DrawArgs["quad"].BaseVertexLocation,
			0);
	}


	
#if 0
	NewUIFrame();
	RenderUIDockSpace();
	color_editor.show();
	RenderUI();
#endif

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
void EditorApp::SwapFrameResource()
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

void EditorApp::BuildRootSignature()
{
	// 2 CBV and 1 SRV signature
	{
		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER rootParameters[3] = {};

		// Create root CBV.
		rootParameters[0].InitAsConstantBufferView(0);
		rootParameters[1].InitAsConstantBufferView(1);
	
		CD3DX12_DESCRIPTOR_RANGE texTable;
		texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	
		rootParameters[2].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

		auto staticSamplers = D3DUtil::GetStaticSamplers();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
			3, 
			rootParameters, 
			static_cast<UINT>(staticSamplers.size()), 
			staticSamplers.data(), 
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
}

void EditorApp::BuildShadersAndInputLayout()
{
	// default
	{
		ComPtr<ID3DBlob> vsBlob;
		ThrowIfFailed(D3DReadFileToBlob(L"DefaultVS.cso", &vsBlob));
		_Shaders.insert(std::pair("default_vs", vsBlob));

		ComPtr<ID3DBlob> psBlob;
		ThrowIfFailed(D3DReadFileToBlob(L"DefaultPS.cso", &psBlob));
		_Shaders.insert(std::pair("default_ps", psBlob));
	}

	// quad
	{
		ComPtr<ID3DBlob> vsBlob;
		ThrowIfFailed(D3DReadFileToBlob(L"QuadVS.cso", &vsBlob));
		_Shaders.insert(std::pair("quad_vs", vsBlob));

		ComPtr<ID3DBlob> psBlob;
		ThrowIfFailed(D3DReadFileToBlob(L"QuadPS.cso", &psBlob));
		_Shaders.insert(std::pair("quad_ps", psBlob));
	}

	_InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void EditorApp::BuildPSO()
{
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
		//defaultPSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		defaultPSO.SampleMask = UINT_MAX;
		defaultPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		defaultPSO.NumRenderTargets = 1;
		defaultPSO.RTVFormats[0] = _BackBufferFormat;
		defaultPSO.SampleDesc.Count = _4xMsaaState ? 4 : 1;
		defaultPSO.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
		//defaultPSO.DSVFormat = _DepthStencilFormat;
	
		ThrowIfFailed(_Device->CreateGraphicsPipelineState(&defaultPSO, IID_PPV_ARGS(&_PSOs["render_target"])));
	}

	// render target Quad
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		psoDesc.InputLayout = { _InputLayout.data(), (UINT)_InputLayout.size() };
		psoDesc.pRootSignature = _RootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(_Shaders["quad_vs"]->GetBufferPointer()),
			_Shaders["quad_vs"]->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(_Shaders["quad_ps"]->GetBufferPointer()),
			_Shaders["quad_ps"]->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = _BackBufferFormat;
		psoDesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
		psoDesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = _DepthStencilFormat;

		ThrowIfFailed(_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_PSOs["back_buffer"])));
	}


}

void EditorApp::LoadDefaultMeshes()
{
	// Box
	{
		std::array<Vertex, 8> vertices =
		{
			Vertex( XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
			Vertex( XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
			Vertex( XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
			Vertex( XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
			Vertex( XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
			Vertex( XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
			Vertex( XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
			Vertex( XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f,0.f) ),
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

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "box_geo";

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(
			_Device.Get(),
			_CommandList.Get(),
			vertices.data(),
			vbByteSize,
			geo->VertexBufferUploader);

		geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(
			_Device.Get(),
			_CommandList.Get(),
			indices.data(),
			ibByteSize,
			geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs["box"] = submesh;

		_Meshes[geo->Name] = std::move(geo);
	}

	// Quad
	{
		std::array<Vertex, 4> vertices =
		{
			Vertex( XMFLOAT3(0.0f,-1.0f, 0.0f), XMFLOAT3(0.f, 0.f, -1.f), XMFLOAT2(0.f,1.f)),
			Vertex( XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.f, 0.f, -1.f), XMFLOAT2(0.f,0.f)),
			Vertex( XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.f, 0.f, -1.f), XMFLOAT2(1.f,0.f)),
			Vertex( XMFLOAT3(1.0f,-1.0f, 0.0f), XMFLOAT3(0.f, 0.f, -1.f), XMFLOAT2(1.f,1.f)),
		};

		std::array<std::uint16_t, 6> indices =
		{
			0,1,2,
			0,2,3
		};

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "quad_geo";

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(
			_Device.Get(),
			_CommandList.Get(),
			vertices.data(),
			vbByteSize,
			geo->VertexBufferUploader);

		geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(
			_Device.Get(),
			_CommandList.Get(),
			indices.data(),
			ibByteSize,
			geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs["quad"] = submesh;

		_Meshes[geo->Name] = std::move(geo);
	}

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