#pragma once

#include "Rendering\Mesh.h"
#include <DirectXMath.h>

struct Entity
{
	uint32_t Id;
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMFLOAT3 Color; // TODO: temporary
	Mesh* Mesh;
	Submesh Submesh;
};