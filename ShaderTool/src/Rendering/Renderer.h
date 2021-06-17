#pragma once

#include "Editor\ImGui\imgui.h"
#include "Editor\ImGui\imgui_impl_win32.h"
#include "Editor\ImGui\imgui_impl_dx12.h"
#include "Editor\ImGui\imnodes.h"

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
	bool InitImGui();
	void OnResize(uint32_t width, uint32_t height);
	void CreateCommandObjects();
	void CreateRenderTargetViews();
	void CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height);
	void CreateRTVAndDSVDescriptorHeaps();
	void CreateDSVBuffer();
	void SetViewportAndScissor();
	void Clear();
	void Present();
	void Flush();
	void NewUIFrame();
	void RenderUI();

	bool HasToResizeBuffer(int w, int h) const { return _CurrentBufferWidth != w || _CurrentBufferHeight != h; }
	ID3D12Device* GetDevice() const { return _Device.Get(); }
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return _SRVDescriptorHeap.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Device>              _Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>        _CommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4>           _SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>      _RTVDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>      _DSVDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>		      _BackBuffers[NUM_FRAMES];
	Microsoft::WRL::ComPtr<ID3D12Resource>			  _DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    _CommandAllocators[NUM_FRAMES];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _CommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence>				  _Fence;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	  _SRVDescriptorHeap;

	D3D12_VIEWPORT _ScreenViewport{};
	D3D12_RECT _ScissorRect{};
	DXGI_FORMAT _BackBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT _DepthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };
	bool _4xMsaaState{ false };
	UINT _4xMsaaQuality{ 0 };
	bool _TearingSupport{ false };
	bool _VSync{ true };
	uint16_t _CurrentBackBufferIndex{ 0 };
	uint32_t _RTVDescriptorSize{ 0 };
	uint32_t _DSVDescriptorSize{ 0 };
	uint32_t _CbvSrvUavDescriptorSize{ 0 };
	uint64_t _FenceValue{ 0 };
	HANDLE _FenceEvent{ nullptr };
	//uint64_t _FrameFenceValues[NUM_FRAMES]{ 0 };
	uint32_t _CurrentBufferWidth{ 0 };
	uint32_t _CurrentBufferHeight{ 0 };
	Window* _Window;


};