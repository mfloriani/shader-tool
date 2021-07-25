#pragma once

#include "D3DUtil.h"

class PipelineStateObject
{
public:
	PipelineStateObject(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat, bool msaa4xState, UINT msaa4xQuality);
	~PipelineStateObject();

	void Create(
		ID3D12Device* device,
		ID3D12RootSignature* rootSign,
		D3D12_INPUT_LAYOUT_DESC inputLayout,
		const std::string& vs,
		const std::string& ps,
		bool setDepthStencil = true);

	ID3D12PipelineState* GetPSO() { return _PSO.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _PSO;
	DXGI_FORMAT _BackBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT _DepthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };
	bool _4xMsaaState{ false };
	UINT _4xMsaaQuality{ 0 };

};