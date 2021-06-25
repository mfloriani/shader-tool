#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Defines.h"

using Microsoft::WRL::ComPtr;

Renderer::Renderer(Window* window) :
    _Window(window)
{
    auto [w, h] = window->GetSize();
    _CurrentBufferWidth = w;
    _CurrentBufferHeight = h;
}

Renderer::~Renderer()
{
    FlushCommandQueue();
    
    ImNodes::DestroyContext();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

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

    LOG_TRACE("Tearing suport? {0}", (allowTearing == TRUE));

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

    // Log the chosen adapter 
    DXGI_ADAPTER_DESC adapterDesc{};
    dxgiAdapter4->GetDesc(&adapterDesc);
    LOG_TRACE(L"Adapter {0}", adapterDesc.Description);

    return dxgiAdapter4;
}

void Renderer::CreateSwapChain( HWND hWnd, uint32_t width, uint32_t height )
{
    LOG_TRACE("SwapChain size x: {0}, y: {1}", width, height);

    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = _BackBufferFormat;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = NUM_BACK_BUFFERS;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    if (::CheckTearingSupport())
        swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        _CommandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1)
    );

    // Disable the Alt+Enter fullscreen toggle feature. 
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&_SwapChain));
}

void Renderer::CreateRTVAndDSVDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = NUM_BACK_BUFFERS;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(_Device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS(_RTVDescriptorHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(_Device->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS(_DSVDescriptorHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_SRVDescriptorHeap)));
}

void Renderer::CreateRenderTargetViews()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (int i = 0; i < NUM_BACK_BUFFERS; ++i)
    {
        ThrowIfFailed(_SwapChain->GetBuffer(i, IID_PPV_ARGS(&_BackBuffers[i])));
        _Device->CreateRenderTargetView(_BackBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, _RTVDescriptorSize);
    }
}

void Renderer::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;
    ThrowIfFailed(_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_CommandQueue)));

    for (int i = 0; i < NUM_FRAMES; ++i)
        ThrowIfFailed(
            _Device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, 
                IID_PPV_ARGS(&_FrameResources[i]->CmdListAlloc)
            )
        );

    _CurrFrameResourceIndex = 0;
    _CurrFrameResource = _FrameResources[_CurrFrameResourceIndex].get();

    ThrowIfFailed(
        _Device->CreateCommandList(
            0, 
            D3D12_COMMAND_LIST_TYPE_DIRECT, 
            _CurrFrameResource->CmdListAlloc.Get(),
            nullptr, 
            IID_PPV_ARGS(&_CommandList)
        )
    );
    ThrowIfFailed(_CommandList->Close());
    ThrowIfFailed(_CommandList->Reset(_CurrFrameResource->CmdListAlloc.Get(), nullptr));
}

void Renderer::FlushCommandQueue()
{
    ++_CurrentFenceValue;
    ThrowIfFailed(_CommandQueue->Signal(_Fence.Get(), _CurrentFenceValue));

    if (_Fence->GetCompletedValue() < _CurrentFenceValue)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(_Fence->SetEventOnCompletion(_CurrentFenceValue, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

}

void Renderer::NewUIFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Renderer::CreateDSVBuffer()
{
    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = _CurrentBufferWidth;
    depthStencilDesc.Height = _CurrentBufferHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = _DepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(_DepthStencilBuffer.GetAddressOf())));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = _DepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    _Device->CreateDepthStencilView(_DepthStencilBuffer.Get(), &dsvDesc, _DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Transition the resource from its initial state to be used as a depth buffer.
    _CommandList->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            _DepthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        )
    );

    // Execute the resize commands.
    ThrowIfFailed(_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = { _CommandList.Get() };
    _CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void Renderer::SetViewportAndScissor()
{
    // Update the viewport transform to cover the client area.
    _ScreenViewport.TopLeftX = 0;
    _ScreenViewport.TopLeftY = 0;
    _ScreenViewport.Width = static_cast<float>(_CurrentBufferWidth);
    _ScreenViewport.Height = static_cast<float>(_CurrentBufferHeight);
    _ScreenViewport.MinDepth = 0.0f;
    _ScreenViewport.MaxDepth = 1.0f;

    _ScissorRect = { 0, 0, static_cast<LONG>(_CurrentBufferWidth), static_cast<LONG>(_CurrentBufferHeight) };
}

bool Renderer::Init()
{
    LOG_TRACE("Renderer::Init()");
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

    _TearingSupport = ::CheckTearingSupport();
    
    ComPtr<IDXGIAdapter4> adapter = ::GetAdapter(false);
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_Device)));

    _RTVDescriptorSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    _DSVDescriptorSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    _CbvSrvUavDescriptorSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Check 4X MSAA quality support for our back buffer format.
    // All Direct3D 11 capable devices support 4X MSAA for all render
    // target formats, so we only need to check quality support.
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = _BackBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(_Device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels)));
    _4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");


    for (int i = 0; i < NUM_FRAMES; ++i)
        _FrameResources[i] = std::make_unique<FrameResource>(_Device.Get(), 1, 0);

    CreateCommandObjects();

    CreateSwapChain( _Window->GetHandler(), _CurrentBufferWidth, _CurrentBufferHeight);
    _CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

    CreateRTVAndDSVDescriptorHeaps();
    CreateRenderTargetViews();

    ThrowIfFailed(_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_Fence)));

    CreateDSVBuffer();
    SetViewportAndScissor();
    
    FlushCommandQueue();

    if (!InitGUI())
        return false;    

    ::ShowWindow(_Window->GetHandler(), SW_SHOW);
    ::UpdateWindow(_Window->GetHandler());

	return true;
}

bool Renderer::InitGUI()
{
    LOG_TRACE("Renderer::InitImGui()");
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();    
    ImGui::CreateContext();
    ImNodes::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    if (!ImGui_ImplWin32_Init(_Window->GetHandler()))
    {
        LOG_CRITICAL("Error ImGui_ImplWin32_Init");
        return false;
    }
    if (!ImGui_ImplDX12_Init(
        _Device.Get(),
        NUM_BACK_BUFFERS,
        _BackBufferFormat,
        _SRVDescriptorHeap.Get(),
        _SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        _SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
    ))
    {
        LOG_CRITICAL("Error ImGui_ImplDX12_Init");
        return false;
    }

    return true;
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    // Don't allow 0 size swap chain back buffers.
    _CurrentBufferWidth = std::max(1u, width);
    _CurrentBufferHeight = std::max(1u, height);

    LOG_TRACE("OnResize x: {0}, y: {1}", _CurrentBufferWidth, _CurrentBufferHeight);

    // Flush the GPU queue to make sure the swap chain's back buffers
    // are not being referenced by an in-flight command list.
    FlushCommandQueue();

    auto commandAllocator = _CurrFrameResource->CmdListAlloc;
    _CommandList->Reset(commandAllocator.Get(), nullptr);

    for (int i = 0; i < NUM_BACK_BUFFERS; ++i)
        _BackBuffers[i].Reset();

    _DepthStencilBuffer.Reset();

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    ThrowIfFailed(_SwapChain->GetDesc(&swapChainDesc));
    ThrowIfFailed(
        _SwapChain->ResizeBuffers(
            NUM_BACK_BUFFERS, 
            _CurrentBufferWidth,
            _CurrentBufferHeight,
            swapChainDesc.BufferDesc.Format, 
            swapChainDesc.Flags
        )
    );

    _CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

    CreateRenderTargetViews();
    CreateDSVBuffer();
    SetViewportAndScissor();
    FlushCommandQueue();
}

void Renderer::Clear()
{
    auto commandAllocator = _CurrFrameResource->CmdListAlloc;
    auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];

    commandAllocator->Reset();
    _CommandList->Reset(commandAllocator.Get(), nullptr);

    // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
    _CommandList->RSSetViewports(1, &_ScreenViewport);
    _CommandList->RSSetScissorRects(1, &_ScissorRect);
    
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer.Get(),
        D3D12_RESOURCE_STATE_PRESENT, 
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _CommandList->ResourceBarrier(1, &barrier);

    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
        _RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        _CurrentBackBufferIndex, 
        _RTVDescriptorSize
    );

    auto& dsv = _DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    _CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    _CommandList->ClearDepthStencilView(
        dsv, 
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 
        1.0f, 
        0, 0, 
        nullptr
    );

    // Specify the buffers we are going to render to.
    _CommandList->OMSetRenderTargets(1, &rtv, true, &dsv);

}

void Renderer::Present()
{
    auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];
    
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, 
        D3D12_RESOURCE_STATE_PRESENT
    );
    _CommandList->ResourceBarrier(1, &barrier);

    ThrowIfFailed(_CommandList->Close());

    ID3D12CommandList* const commandLists[] = { _CommandList.Get() };
    _CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    UINT syncInterval = _VSync ? 1 : 0;
    UINT presentFlags = _TearingSupport && !_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(_SwapChain->Present(syncInterval, presentFlags));

    _CurrentBackBufferIndex = (_CurrentBackBufferIndex + 1) % NUM_BACK_BUFFERS;

    //Flush();   

    // Advance the fence value to mark commands up to this fence point.
    _CurrFrameResource->FenceValue = ++_CurrentFenceValue;

    // Add an instruction to the command queue to set a new fence point.
    // Because we are on the GPU timeline, the new fence point won't be
    // set until the GPU finishes processing all the commands prior to this Signal().
    _CommandQueue->Signal(_Fence.Get(), _CurrentFenceValue);

}

void Renderer::RenderUI()
{
    ImGui::Render();

    ID3D12DescriptorHeap* const descHeapList[] = { _SRVDescriptorHeap.Get() };
    _CommandList->SetDescriptorHeaps(1, descHeapList);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _CommandList.Get());

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(NULL, (void*)_CommandList.Get());
    }
}

void Renderer::SwapFrameResource()
{
    // cycle through the frames to continue writing commands to avoid having the GPU idle
    _CurrFrameResourceIndex = (_CurrFrameResourceIndex + 1) % NUM_FRAMES;
    _CurrFrameResource = _FrameResources[_CurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if (_CurrFrameResource->FenceValue != 0 && _Fence->GetCompletedValue() < _CurrFrameResource->FenceValue)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(_Fence->SetEventOnCompletion(_CurrFrameResource->FenceValue, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}
