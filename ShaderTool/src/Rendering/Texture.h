#pragma once

#include "D3DUtil.h"
#include <string>

struct Texture
{
	std::string Name;
	std::wstring Filename;
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap{ nullptr };
	CD3DX12_CPU_DESCRIPTOR_HANDLE SrvCpuDescHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGpuDescHandle;
};

