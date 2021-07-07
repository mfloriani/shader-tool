#include "pch.h"
#include "RenderTexture.h"

using namespace D3DUtil;

RenderTexture::RenderTexture(
    ID3D12Device* device,
    DXGI_FORMAT format,
    uint32_t w, 
    uint32_t h
    ) noexcept :
    _Device(device),
    _ClearColor{},
    _Format(format),
    _Width(w),
    _Height(h)
{
    CreateViewportScissor();
    CreateResource();
}

RenderTexture::~RenderTexture() noexcept
{
    
}

void RenderTexture::CreateViewportScissor()
{
    _Viewport.TopLeftX = 0;
    _Viewport.TopLeftY = 0;
    _Viewport.Width = static_cast<float>(_Width);
    _Viewport.Height = static_cast<float>(_Height);
    _Viewport.MinDepth = 0.0f;
    _Viewport.MaxDepth = 1.0f;

    _ScissorRect = { 0, 0, static_cast<LONG>(_Width), static_cast<LONG>(_Height) };
}

void RenderTexture::CreateDescriptors(
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu,
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu,
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu)
{
    _SrvCpuDescHandle = srvCpu;
    _SrvGpuDescHandle = srvGpu;
    _RtvCpuDescHandle = rtvCpu;

    CreateDescriptors();
}

void RenderTexture::CreateDescriptors()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = _Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    _Device->CreateShaderResourceView(_Resource.Get(), &srvDesc, _SrvCpuDescHandle);
    _Device->CreateRenderTargetView(_Resource.Get(), nullptr, _RtvCpuDescHandle);
}

void RenderTexture::OnResize(UINT newWidth, UINT newHeight)
{
    if ((_Width != newWidth) || (_Height != newHeight))
    {
        _Width = newWidth;
        _Height = newHeight;

        CreateResource();
        CreateDescriptors();
    }
}

void RenderTexture::CreateResource()
{
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
        _Format,
        static_cast<UINT64>(_Width),
        static_cast<UINT>(_Height),
        1, 1, 1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue = { _Format, {} };
    memcpy(clearValue.Color, _ClearColor, sizeof(clearValue.Color));

    ThrowIfFailed(
        _Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            &clearValue,
            IID_PPV_ARGS(_Resource.ReleaseAndGetAddressOf()))
    );
    _Resource->SetName(L"RenderTexture");
}
