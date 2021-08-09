#pragma once

struct Model
{
	std::string Name;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount;
	UINT StartIndexLocation;
	UINT BaseVertexLocation;
};