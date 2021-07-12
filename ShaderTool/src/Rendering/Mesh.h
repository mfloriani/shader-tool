#pragma once

#include "D3DUtil.h"

#include <unordered_map>

// Offsets for multiple meshes stored in one vertex and index buffer.
struct Submesh
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
};

struct Mesh
{
	std::string Name;

	// System memory copies. Use Blobs because the vertex/index format can be generic.	
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	//DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R32_UINT;
	UINT IndexBufferByteSize = 0;

	// A Mesh may store multiple meshes in one vertex/index buffer.
	// Use this container to define the Submeshes so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, Submesh> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};