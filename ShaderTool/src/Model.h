#pragma once

struct Model
{
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount;
	UINT StartIndexLocation;
	UINT BaseVertexLocation;
};