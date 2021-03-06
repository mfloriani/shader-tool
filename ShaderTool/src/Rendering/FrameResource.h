#pragma once

//***************************************************************************************
// Code adapted from Frank Luna book about DX12
//***************************************************************************************

#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl.h>

#include "UploadBuffer.h"
#include "PipelineStateObject.h"
#include "Texture.h"

//struct ObjectConstants
//{
//    DirectX::XMFLOAT4X4 World{ D3DUtil::Identity4x4() };
//};

//struct FrameConstants
//{
//    DirectX::XMFLOAT4X4 View{ D3DUtil::Identity4x4() };
//    DirectX::XMFLOAT4X4 Proj{ D3DUtil::Identity4x4() };
    
    //DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    //DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    //DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    //DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    //DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    //DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    //DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    //float cbPerObjectPad1 = 0.0f;
    //DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    //DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    //float NearZ = 0.0f;
    //float FarZ = 0.0f;
    //float TotalTime = 0.0f;
    //float DeltaTime = 0.0f;
//};

// Stores the resources needed for the CPU to build the command lists for a frame.  
struct FrameResource
{
    FrameResource(ID3D12Device* device, UINT frameCount, UINT objectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.
    //std::unique_ptr<UploadBuffer<FrameConstants>> FrameCB = nullptr;
    //std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

    std::shared_ptr<PipelineStateObject> RenderTargetPSO;

    std::shared_ptr<Texture> RenderTargetSrvTexture;

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 FenceValue = 0;
};