#include "pch.h"
#include "ShaderToolApp.h"
#include "GeometryGenerator.h"
#include "AssetManager.h"

using namespace DirectX;
using namespace D3DUtil;

using Microsoft::WRL::ComPtr;

bool ShaderToolApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// TODO: review this render-texture perspective
	// The window resized, so update the aspect ratio and recompute the projection matrix.
	UINT width = 1024;
	UINT height = 1024;
	auto aspectRatio = static_cast<float>(width) / height;
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio, 1.0f, 1000.0f);
	XMStoreFloat4x4(&_Proj, P);

	// Render texture used by the render target node
	_RenderTarget = std::make_unique<RenderTexture>(
		_Device.Get(),
		_BackBufferFormat,
		width,
		height
		);

	auto commandAllocator = _CurrFrameResource->CmdListAlloc;
	_CommandList->Reset(commandAllocator.Get(), nullptr);

	CreateDescriptorHeaps();
	BuildBackBufferPSO();
	LoadPrimitiveModels();

	ThrowIfFailed(_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { _CommandList.Get() };
	_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	_Timer.Reset();
	ImNodesIO& io = ImNodes::GetIO();
	io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

	// TODO: move this to the proper place
	_Entity.Id = 0;
	_Entity.Model = AssetManager::Get().GetModel("cube");
	_Entity.Position = {0.f, 0.f, 0.f};
	_Entity.Scale = {10.f, 10.f, 10.f};
	_Entity.Rotation = {0.f, 0.f, 0.f};
	
	InitNodeGraph();

	return true;
}

void ShaderToolApp::InitNodeGraph()
{
	ShaderManager::Get().LoadShaderFromFile("default.fx");

	// TODO: testing
	//auto vsQuad = shaderMgr.LoadShaderFromFile("C:\\Users\\muril\\Downloads\\quad.fx", ShaderType::Vertex);
	//auto psQuad = shaderMgr.LoadShaderFromFile("C:\\Users\\muril\\Downloads\\quad.fx", ShaderType::Pixel);

	CreateRenderTargetPSO(NOT_LINKED);
}

void ShaderToolApp::CreateDescriptorHeaps()
{
	_RenderTarget->CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_ImGuiSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, _CbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_ImGuiSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 1, _CbvSrvUavDescriptorSize),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), NUM_BACK_BUFFERS, _RtvDescriptorSize)
	);

}

void ShaderToolApp::BuildBackBufferPSO()
{
	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		0,
		nullptr,
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
			IID_PPV_ARGS(_BackBufferRootSignature.GetAddressOf())));
	

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	ComPtr<ID3DBlob> bytecode;
	ShaderManager::Get().LoadBinaryShader(BACKBUFFER_VS, bytecode);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso;
	ZeroMemory(&pso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pso.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	pso.pRootSignature = _BackBufferRootSignature.Get();
	pso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pso.SampleMask = UINT_MAX;
	pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso.NumRenderTargets = 1;
	pso.RTVFormats[0] = _BackBufferFormat;
	pso.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	pso.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
	pso.DSVFormat = _DepthStencilFormat;

	pso.VS = {
		reinterpret_cast<BYTE*>(bytecode->GetBufferPointer()),
		bytecode->GetBufferSize()
	};
	
	ThrowIfFailed(_Device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&_BackBufferPSO)));

}

void ShaderToolApp::LoadPrimitiveModels()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData cube = geoGen.CreateBox(1.f, 1.f, 1.f, 0u);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.f, 20u, 20u);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(10.f, 10.f, 10u, 10u);

	auto mesh = std::make_shared<Mesh>();
	mesh->Name = "primitives";

	// the order that vertices and indices are inserted MUST be the same as the offsets in the submeshes

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

	AssetManager::Get().AddMesh(mesh);

	// The meshes MUST be stored and in the exact same way they are indexed in the submeshes

	Model model;
	model.VertexBufferView = mesh->VertexBufferView();
	model.IndexBufferView = mesh->IndexBufferView();

	// Cube
	model.IndexCount = (UINT)cube.Indices32.size();
	model.StartIndexLocation = 0;
	model.BaseVertexLocation = 0;
	AssetManager::Get().AddModel("cube", model);
	_Primitives.push_back("cube");

	// Sphere
	model.IndexCount = (UINT)sphere.Indices32.size();
	model.StartIndexLocation = (UINT)cube.Indices32.size();
	model.BaseVertexLocation = (UINT)cube.Vertices.size();
	AssetManager::Get().AddModel("sphere", model);
	_Primitives.push_back("sphere");

	// Grid
	model.IndexCount = (UINT)grid.Indices32.size();
	model.StartIndexLocation += (UINT)sphere.Indices32.size();
	model.BaseVertexLocation += (UINT)sphere.Vertices.size();
	AssetManager::Get().AddModel("grid", model);
	_Primitives.push_back("grid");
}