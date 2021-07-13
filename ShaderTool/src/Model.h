#pragma once

struct Model
{
	//D3D12_VERTEX_BUFFER_VIEW GetVBV() const { return _Mesh->VertexBufferView(); }
	//D3D12_INDEX_BUFFER_VIEW GetIBV() const { return _Mesh->IndexBufferView(); }
	//UINT GetIndexCount() const { return _IndexCount; }
	//UINT GetStartIndexLocation() const { return _StartIndexLocation; }
	//UINT GetBaseVertexLocation() const { return _BaseVertexLocation; }

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount;
	UINT StartIndexLocation;
	UINT BaseVertexLocation;
};