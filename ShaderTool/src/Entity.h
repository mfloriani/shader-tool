#pragma once

#include "Model.h"

#include <DirectXMath.h>

struct Entity
{
	uint32_t Id;
	Model Model;
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
};