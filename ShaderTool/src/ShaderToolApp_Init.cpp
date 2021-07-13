#include "pch.h"
#include "ShaderToolApp.h"
#include "GeometryGenerator.h"

using namespace DirectX;
using namespace D3DUtil;

using Microsoft::WRL::ComPtr;

bool ShaderToolApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	UINT width = 1024;
	UINT height = 1024;
	auto aspectRatio = static_cast<float>(width) / height;
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio, 1.0f, 1000.0f);
	XMStoreFloat4x4(&_RTProj, P);

	// RENDER-TO-TEXTURE
	_RenderTarget = std::make_unique<RenderTexture>(
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
	LoadPrimitiveMeshes();

	ThrowIfFailed(_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { _CommandList.Get() };
	_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	// Node graph
	_Timer.Reset();
	ImNodesIO& io = ImNodes::GetIO();
	io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

	// TODO: move this to the proper place
	_Entity.Id = 0;
	_Entity.Mesh = _Meshes["primitives"].get();
	_Entity.Submesh = _Entity.Mesh->DrawArgs["cube"];
	_Entity.Position = {0.f, 0.f, 0.f};
	_Entity.Scale = {10.f, 10.f, 10.f};
	_Entity.Rotation = {0.f, 0.f, 0.f};
	
	return true;
}

void ShaderToolApp::CreateDescriptorHeaps()
{
	_RenderTarget->CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_ImGuiSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, _CbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_ImGuiSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 1, _CbvSrvUavDescriptorSize),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), NUM_BACK_BUFFERS, _RtvDescriptorSize)
	);

}

void ShaderToolApp::BuildRootSignature()
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

void ShaderToolApp::BuildShadersAndInputLayout()
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
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void ShaderToolApp::BuildPSO()
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

void ShaderToolApp::LoadPrimitiveMeshes()
{
	GeometryGenerator geoGen;

	GeometryGenerator::MeshData cube = geoGen.CreateBox(1.f, 1.f, 1.f, 0u);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.f, 20u, 20u);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(10.f, 10.f, 10u, 10u);

	auto mesh = std::make_unique<Mesh>();
	mesh->Name = "primitives";

	// The meshes MUST be stored and in the exact same way they are indexed in the submeshes

	Submesh cubeSubmesh;
	cubeSubmesh.IndexCount = (UINT)cube.Indices32.size();
	cubeSubmesh.StartIndexLocation = 0;
	cubeSubmesh.BaseVertexLocation = 0;
	mesh->DrawArgs["cube"] = cubeSubmesh;

	Submesh sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = (UINT)cube.Indices32.size();
	sphereSubmesh.BaseVertexLocation = (UINT)cube.Vertices.size();
	mesh->DrawArgs["sphere"] = sphereSubmesh;

	Submesh gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = sphereSubmesh.StartIndexLocation + (UINT)sphere.Indices32.size();
	gridSubmesh.BaseVertexLocation = sphereSubmesh.BaseVertexLocation + (UINT)sphere.Vertices.size();
	mesh->DrawArgs["grid"] = gridSubmesh;

	// the order that vertices and indices are inserted MUST be the same as the index offsets set in the submeshes

	std::vector<Vertex> vertices;
	vertices.insert(vertices.end(), cube.Vertices.begin(), cube.Vertices.end());
	vertices.insert(vertices.end(), sphere.Vertices.begin(), sphere.Vertices.end());
	vertices.insert(vertices.end(), grid.Vertices.begin(), grid.Vertices.end());

	std::vector<uint32_t> indices;
	indices.insert(indices.end(), cube.Indices32.begin(), cube.Indices32.end());
	indices.insert(indices.end(), sphere.Indices32.begin(), sphere.Indices32.end());
	indices.insert(indices.end(), grid.Indices32.begin(), grid.Indices32.end());

	const UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
	const UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(uint32_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mesh->VertexBufferCPU));
	CopyMemory(mesh->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mesh->IndexBufferCPU));
	CopyMemory(mesh->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mesh->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(
		_Device.Get(),
		_CommandList.Get(),
		vertices.data(),
		vbByteSize,
		mesh->VertexBufferUploader);

	mesh->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(
		_Device.Get(),
		_CommandList.Get(),
		indices.data(),
		ibByteSize,
		mesh->IndexBufferUploader);

	mesh->VertexByteStride = sizeof(Vertex);
	mesh->VertexBufferByteSize = vbByteSize;
	mesh->IndexFormat = DXGI_FORMAT_R32_UINT;
	mesh->IndexBufferByteSize = ibByteSize;

	_Meshes[mesh->Name] = std::move(mesh);

}