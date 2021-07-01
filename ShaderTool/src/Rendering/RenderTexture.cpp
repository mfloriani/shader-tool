#include "pch.h"
#include "RenderTexture.h"

using namespace D3DUtil;

RenderTexture::RenderTexture(DXGI_FORMAT format) noexcept :
    _State(D3D12_RESOURCE_STATE_COMMON),
    _SrvDescHandle{},
    _RtvDescHandle{},
    _ClearColor{},
    _Format(format),
    _Width(0),
    _Height(0)
{
}

RenderTexture::~RenderTexture() noexcept
{
    Release();
}

void RenderTexture::Release()
{
    _Resource.Reset();
    _Device.Reset();

    _State = D3D12_RESOURCE_STATE_COMMON;
    _Width = 0;
    _Height = 0;
    _SrvDescHandle.ptr = 0;
    _RtvDescHandle.ptr = 0;
}

bool RenderTexture::Init(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvDescHandle, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle)
{
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { _Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
        {
            LOG_CRITICAL("RenderTexture::Init -> Error checking feature suport");
            return false;
        }

        UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
        if ((formatSupport.Support1 & required) != required)
        {
            LOG_CRITICAL("RenderTexture::Init -> Device does not support the requested format {0}!", _Format);
            return false;
        }
    }

    if (!srvDescHandle.ptr || !rtvDescHandle.ptr)
    {
        LOG_CRITICAL("RenderTexture::Init -> Invalid descriptors");
        return false;
    }

    _Device = device;
    _SrvDescHandle = srvDescHandle;
    _RtvDescHandle = rtvDescHandle;

    return true;
}

void RenderTexture::SetSize(uint32_t w, uint32_t h)
{
    if (w == _Width && h == _Height)
        return;

    if (!_Device)
    {
        LOG_ERROR("RenderTexture::OnResize -> Device is not set");
        return;
    }

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
        _Format,
        static_cast<UINT64>(w),
        static_cast<UINT>(h),
        1, 1, 1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue = { _Format, {} };
    memcpy(clearValue.Color, _ClearColor, sizeof(clearValue.Color));

    _State = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Create a render target
    ThrowIfFailed(
        _Device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            _State,
            &clearValue,
            IID_PPV_ARGS(_Resource.ReleaseAndGetAddressOf()))
    );
    _Resource->SetName(L"RenderTexture");

    _Device->CreateRenderTargetView(_Resource.Get(), nullptr, _RtvDescHandle);
    _Device->CreateShaderResourceView(_Resource.Get(), nullptr, _SrvDescHandle);

    _Width = w;
    _Height = h;
}

void RenderTexture::Clear(ID3D12GraphicsCommandList* cmdList)
{
    cmdList->ClearRenderTargetView(_RtvDescHandle, _ClearColor, 0, nullptr);
}

void RenderTexture::BeginRender(ID3D12GraphicsCommandList* cmdList)
{
    TransitionTo(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void RenderTexture::EndRender(ID3D12GraphicsCommandList* cmdList)
{
    TransitionTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void RenderTexture::TransitionTo(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES afterState)
{
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_Resource.Get(), _State, afterState);
    cmdList->ResourceBarrier(1, &barrier);
    _State = afterState;
}
