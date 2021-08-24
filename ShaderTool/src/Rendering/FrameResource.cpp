#include "pch.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT frameCount, UINT objectCount)
{
    ThrowIfFailed(
        device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(CmdListAlloc.GetAddressOf())
        )
    );

    //FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, frameCount, true);
    //ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);

    RenderTargetSrvTexture = std::make_shared<Texture>();

}

FrameResource::~FrameResource()
{
}
