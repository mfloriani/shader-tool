#pragma once

#include "D3DUtil.h"

class RenderTexture
{
public:
	RenderTexture(DXGI_FORMAT format) noexcept;
	RenderTexture(const RenderTexture&) = delete;
	RenderTexture& operator=(const RenderTexture&) = delete;
	~RenderTexture() noexcept;

	bool Init(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvDescHandle, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle);
	void Release();
	void SetSize(uint32_t w, uint32_t h);
	void Clear(ID3D12GraphicsCommandList* cmdList);
	void BeginRender(ID3D12GraphicsCommandList* cmdList);
	void EndRender(ID3D12GraphicsCommandList* cmdList);

	void SetClearColor(DirectX::XMVECTORF32 newColor) { _ClearColor = newColor; }
	void TransitionTo(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES afterState);

private:
	Microsoft::WRL::ComPtr<ID3D12Device>   _Device;
	Microsoft::WRL::ComPtr<ID3D12Resource> _Resource;
	D3D12_RESOURCE_STATES				   _State;
	D3D12_CPU_DESCRIPTOR_HANDLE			   _SrvDescHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			   _RtvDescHandle;
	
	DirectX::XMVECTORF32 _ClearColor;
	DXGI_FORMAT _Format;
	uint32_t _Width;
	uint32_t _Height;
};