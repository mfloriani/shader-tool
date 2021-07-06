#pragma once

#include "D3DUtil.h"

class RenderTexture
{
public:
	RenderTexture(ID3D12Device* device, DXGI_FORMAT format, uint32_t w, uint32_t h) noexcept;
	RenderTexture(const RenderTexture&) = delete;
	RenderTexture& operator=(const RenderTexture&) = delete;
	~RenderTexture() noexcept;

	void OnResize(UINT newWidth, UINT newHeight);	
	void CreateDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu, CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu, CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu);
	
	void SetClearColor(DirectX::XMVECTORF32 newColor) { _ClearColor = newColor; }
	void TransitionTo(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES afterState);

	DirectX::XMVECTORF32 GetClearColor() const { return _ClearColor; }
	ID3D12Resource* GetResource() const { return _Resource.Get(); }
	CD3DX12_CPU_DESCRIPTOR_HANDLE RTV() const { return _RtvCpuDescHandle; }
	CD3DX12_GPU_DESCRIPTOR_HANDLE SRV() const { return _SrvGpuDescHandle; }

private:
	void CreateDescriptors();
	void CreateResource();

private:
	Microsoft::WRL::ComPtr<ID3D12Device>   _Device;
	Microsoft::WRL::ComPtr<ID3D12Resource> _Resource;
	D3D12_RESOURCE_STATES				   _State;
	CD3DX12_CPU_DESCRIPTOR_HANDLE		   _SrvCpuDescHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE		   _SrvGpuDescHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE		   _RtvCpuDescHandle;
	
	DirectX::XMVECTORF32 _ClearColor;
	DXGI_FORMAT _Format;
	UINT _Width;
	UINT _Height;
};