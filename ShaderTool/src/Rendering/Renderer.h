#pragma once

#include "D3DUtil.h"
#include "Defines.h"


#include <stdint.h>

class Window;

class Renderer
{
public:
	Renderer(Window* window);
	~Renderer();

	bool Init();
	void OnResize(uint32_t width, uint32_t height);
	
	void Clear();
	void Present();

private:
	Microsoft::WRL::ComPtr<ID3D12Device>              _Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>        _CommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4>           _SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>      _RTVDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>		      _BackBuffers[NumBuffers];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    _CommandAllocators[NumBuffers];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _CommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence>				  _Fence;

	
	bool _TearingSupport{ false };
	bool _VSync{ true };
	uint16_t _CurrentBackBufferIndex{ 0 };
	uint32_t _RTVDescriptorSize{ 0 };
	uint32_t _DSVDescriptorSize{ 0 };
	uint32_t _CbvSrvUavDescriptorSize{ 0 };
	uint64_t _FenceValue{ 0 };
	HANDLE _FenceEvent;
	uint64_t _FrameFenceValues[NumBuffers]{0};
	uint32_t _ClientWidth{0};
	uint32_t _ClientHeight{0};
	Window* _Window;


};