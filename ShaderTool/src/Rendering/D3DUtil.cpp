#include "pch.h"
#include "D3DUtil.h"

#include <comdef.h>

using namespace D3DUtil;
using Microsoft::WRL::ComPtr;

bool D3DUtil::CompileShader(
    const std::string& filename, 
    const std::string& entryPoint, 
    const std::string& target,
    ComPtr<ID3DBlob>& bytecode)
{
    UINT compileFlags = D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errors;
    auto hr = D3DCompileFromFile(
        AnsiToWString(filename).c_str(),
        nullptr, 
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(), 
        target.c_str(), 
        compileFlags, 
        0, 
        &bytecode, 
        &errors);

    if (errors)
    {
        auto message = std::string((char*)errors->GetBufferPointer());
        if(hr == S_OK)
            LOG_WARN("Shader compiled with warnings [{0}]", message);
        else
        {
            LOG_ERROR("Failed to compile shader [{0}]", message);
            return false;
        }
    }
    else
    {
        LOG_INFO("Shader compiled successfully");
    }

    return true;
}

std::string D3DUtil::ExtractFilename(const std::string& path, bool includeExtension)
{
    std::string filenameWithExtension = path.substr(path.find_last_of("/\\") + 1);
    if (!includeExtension)
    {
        std::string::size_type const p(filenameWithExtension.find_last_of('.'));
        std::string filenameWithoutExtension = filenameWithExtension.substr(0, p);
        return filenameWithoutExtension;
    }

    return filenameWithExtension;
}

UINT D3DUtil::CalcConstantBufferByteSize(UINT byteSize)
{
    // Constant buffers must be a multiple of the minimum hardware
    // allocation size (usually 256 bytes).  So round up to nearest
    // multiple of 256.  We do this by adding 255 and then masking off
    // the lower 2 bytes which store all bits < 256.
    // Example: Suppose byteSize = 300.
    // (300 + 255) & ~255
    // 555 & ~255
    // 0x022B & ~0x00ff
    // 0x022B & 0xff00
    // 0x0200
    // 512
    return (byteSize + 255) & ~255;
}

DirectX::XMFLOAT4X4 D3DUtil::Identity4x4()
{
    static DirectX::XMFLOAT4X4 I(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return I;
}

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
    ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}

std::wstring DxException::ToString()const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

ComPtr<ID3D12Resource> D3DUtil::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    ComPtr<ID3D12Resource>& uploadBuffer)
{
    ComPtr<ID3D12Resource> defaultBuffer;

    // Create the actual default buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    defaultBuffer->SetName(L"DefaultBuffer");

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

    uploadBuffer->SetName(L"UploadBuffer");

    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
    cmdList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, 
            D3D12_RESOURCE_STATE_COPY_DEST)
    );
    
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    
    cmdList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, 
            D3D12_RESOURCE_STATE_GENERIC_READ)
    );

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.

    return defaultBuffer;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> D3DUtil::GetStaticSamplers()
{
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0,																// shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1,																 // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT,		 // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	 // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	 // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2,																// shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,	// filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3,																 // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,	 // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	 // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	 // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4,															 // shaderRegister
        D3D12_FILTER_ANISOTROPIC,				 // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressW
        0.0f,														 // mipLODBias
        8);															 // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5,																// shaderRegister
        D3D12_FILTER_ANISOTROPIC,					// filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressW
        0.0f,															// mipLODBias
        8);																// maxAnisotropy

    return {
        pointWrap, 
        pointClamp,
        linearWrap, 
        linearClamp,
        anisotropicWrap, 
        anisotropicClamp 
    };
}
