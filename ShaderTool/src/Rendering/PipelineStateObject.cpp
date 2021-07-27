#include "pch.h"
#include "PipelineStateObject.h"
#include "ShaderManager.h"

PipelineStateObject::PipelineStateObject(
	DXGI_FORMAT backBufferFormat, 
	DXGI_FORMAT depthStencilFormat, 
	bool msaa4xState, 
	UINT msaa4xQuality)	: 
	_BackBufferFormat(backBufferFormat), 
	_DepthStencilFormat(depthStencilFormat), 
	_4xMsaaState(msaa4xState), 
	_4xMsaaQuality(msaa4xQuality)
{
}

PipelineStateObject::~PipelineStateObject()
{
	LOG_TRACE("PipelineStateObject::~PipelineStateObject()");
}

void PipelineStateObject::Create(
	ID3D12Device* device,
	ID3D12RootSignature* rootSign, 
	D3D12_INPUT_LAYOUT_DESC inputLayout, 
	const std::string& shader)
{
	LOG_INFO("Creating PSO [{0}]", shader);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso;
	ZeroMemory(&pso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pso.InputLayout = inputLayout;
	pso.pRootSignature = rootSign;
	pso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//if (setDepthStencil) pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // TODO: at the moment the render target PSO does not use the depth stencil
	pso.SampleMask = UINT_MAX;
	pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso.NumRenderTargets = 1;
	pso.RTVFormats[0] = _BackBufferFormat;
	pso.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	pso.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
	//if(setDepthStencil) pso.DSVFormat = _DepthStencilFormat; // TODO: at the moment no format is used for render target
	
	auto shaderObj = ShaderManager::Get().GetShader(shader);

	pso.VS = shaderObj->GetVsByteCode();
	pso.PS = shaderObj->GetPsByteCode();

	ThrowIfFailed( device->CreateGraphicsPipelineState( &pso, IID_PPV_ARGS(&_PSO) ));
}
