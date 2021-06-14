#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Defines.h"



using Microsoft::WRL::ComPtr;

bool CheckTearingSupport()
{
    BOOL allowTearing = FALSE;

    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }

    return allowTearing == TRUE;
}

ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (useWarp)
    {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        SIZE_T maxDedicatedVideoMemory = 0;
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
            }
        }
    }

    return dxgiAdapter4;
}

ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
{
    ComPtr<ID3D12Device2> d3d12Device2;
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));

    // Enable debug messages in debug mode.
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
#endif

    return d3d12Device2;
}



ComPtr<IDXGISwapChain4> CreateSwapChain(
    HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue,
    uint32_t width, uint32_t height, uint32_t bufferCount
)
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = ::CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1)
    );

    // Disable the Alt+Enter fullscreen toggle feature. 
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

    return dxgiSwapChain4;
}

ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
    ComPtr<ID3D12Device> device,
    D3D12_DESCRIPTOR_HEAP_TYPE type, 
    uint32_t numDescriptors
)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;

    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

void CreateRenderTargetViews(
    ComPtr<ID3D12Device> device,
    ComPtr<IDXGISwapChain4> swapChain, 
    ComPtr<ID3D12DescriptorHeap> descriptorHeap,
    ComPtr<ID3D12Resource> backBuffers[]
)
{
    auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < NumBuffers; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        backBuffers[i] = backBuffer;

        rtvHandle.Offset(rtvDescriptorSize);
    }
}

ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

    return d3d12CommandQueue;
}

ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(
    ComPtr<ID3D12Device> device,
    D3D12_COMMAND_LIST_TYPE type
)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));
    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> CreateCommandList(
    ComPtr<ID3D12Device> device,
    ComPtr<ID3D12CommandAllocator> commandAllocator, 
    D3D12_COMMAND_LIST_TYPE type
)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
    ThrowIfFailed(commandList->Close());
    return commandList;
}

uint64_t Signal(
    ComPtr<ID3D12CommandQueue> commandQueue, 
    ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue
)
{
    uint64_t fenceValueForSignal = ++fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));
    return fenceValueForSignal;
}

void WaitForFenceValue(
    ComPtr<ID3D12Fence> fence, 
    uint64_t fenceValue, 
    HANDLE fenceEvent,
    std::chrono::milliseconds duration = std::chrono::milliseconds::max()
)
{
    if (fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
        ::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void Flush(
    ComPtr<ID3D12CommandQueue> commandQueue, 
    ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue, 
    HANDLE fenceEvent
)
{
    uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
    WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}

Renderer::Renderer(Window* window) : 
    _Window(window)    
{
    auto [w, h] = window->GetSize();
    _ClientWidth = w;
    _ClientHeight = h;
}

Renderer::~Renderer()
{
    Flush(_CommandQueue, _Fence, _FenceValue, _FenceEvent);

    ::CloseHandle(_FenceEvent);
}

bool Renderer::Init()
{
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

    _TearingSupport = ::CheckTearingSupport();
    LOG_TRACE("Tearing suport? {0}", _TearingSupport);

    ComPtr<IDXGIAdapter4> dxgiAdapter4 = ::GetAdapter(false);
    DXGI_ADAPTER_DESC adapterDesc{};
    dxgiAdapter4->GetDesc(&adapterDesc);
    LOG_TRACE(L"Adapter {0}", adapterDesc.Description);

    _Device = ::CreateDevice(dxgiAdapter4);

    _CommandQueue = ::CreateCommandQueue(_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    LOG_TRACE("Window size x: {0}, y: {1}", _ClientWidth, _ClientHeight);

    _SwapChain = ::CreateSwapChain(
        _Window->GetHandler(), _CommandQueue,
        _ClientWidth, _ClientHeight, NumBuffers
    );

    _CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

    _RTVDescriptorHeap = ::CreateDescriptorHeap(_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NumBuffers);
    _RTVDescriptorSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    
    _DSVDescriptorSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    _CbvSrvUavDescriptorSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ::CreateRenderTargetViews(_Device, _SwapChain, _RTVDescriptorHeap, _BackBuffers);

    for (int i = 0; i < NumBuffers; ++i)
        _CommandAllocators[i] = ::CreateCommandAllocator(_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    _CommandList = ::CreateCommandList(_Device, _CommandAllocators[_CurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

    ThrowIfFailed(_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_Fence)));
    
    _FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);



    ::ShowWindow(_Window->GetHandler(), SW_SHOW);

	return true;
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    if (_ClientWidth != width || _ClientHeight != height)
    {
        // Don't allow 0 size swap chain back buffers.
        _ClientWidth = std::max(1u, width);
        _ClientHeight = std::max(1u, height);

        LOG_TRACE("Window Resize x: {0}, y: {1}", _ClientWidth, _ClientHeight);

        // Flush the GPU queue to make sure the swap chain's back buffers
        // are not being referenced by an in-flight command list.
        Flush(_CommandQueue, _Fence, _FenceValue, _FenceEvent);

        for (int i = 0; i < NumBuffers; ++i)
        {
            // Any references to the back buffers must be released
            // before the swap chain can be resized.
            _BackBuffers[i].Reset();
            _FrameFenceValues[i] = _FrameFenceValues[_CurrentBackBufferIndex];
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(_SwapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(
            _SwapChain->ResizeBuffers(
                NumBuffers, 
                _ClientWidth, 
                _ClientHeight,
                swapChainDesc.BufferDesc.Format, 
                swapChainDesc.Flags
            )
        );

        _CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

        CreateRenderTargetViews(_Device, _SwapChain, _RTVDescriptorHeap, _BackBuffers);
    }
}

void Renderer::Clear()
{
    auto commandAllocator = _CommandAllocators[_CurrentBackBufferIndex];
    auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];

    commandAllocator->Reset();
    _CommandList->Reset(commandAllocator.Get(), nullptr);

    // Clear the render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        _CommandList->ResourceBarrier(1, &barrier);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            _CurrentBackBufferIndex, _RTVDescriptorSize);

        _CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }
}

void Renderer::Present()
{
    auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];

    // Present
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        _CommandList->ResourceBarrier(1, &barrier);

        ThrowIfFailed(_CommandList->Close());

        ID3D12CommandList* const commandLists[] = {
            _CommandList.Get()
        };
        _CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        _FrameFenceValues[_CurrentBackBufferIndex] = Signal(_CommandQueue, _Fence, _FenceValue);

        UINT syncInterval = _VSync ? 1 : 0;
        UINT presentFlags = _TearingSupport && !_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
        ThrowIfFailed(_SwapChain->Present(syncInterval, presentFlags));

        _CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

        WaitForFenceValue(_Fence, _FrameFenceValues[_CurrentBackBufferIndex], _FenceEvent);
    }
}

