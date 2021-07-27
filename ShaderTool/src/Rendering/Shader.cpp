#include "pch.h"
#include "Shader.h"

using Microsoft::WRL::ComPtr;
using namespace D3DUtil;

Shader::Shader(const std::string& name, ComPtr<ID3DBlob> vsBuffer, ComPtr<ID3DBlob> psBuffer)
	: _Name(name), _VsByteCode(vsBuffer), _PsByteCode(psBuffer)
{
	_Reflection = std::make_unique<ShaderReflection>(vsBuffer, psBuffer);
}

Shader::~Shader()
{
	//LOG_TRACE("Shader::~Shader() {0}", _Name);
}

D3D12_SHADER_BYTECODE Shader::GetVsByteCode()
{
	return { 
		reinterpret_cast<BYTE*>(_VsByteCode->GetBufferPointer()), 
		_VsByteCode->GetBufferSize()	
	};
}

D3D12_SHADER_BYTECODE Shader::GetPsByteCode()
{
	return {
		reinterpret_cast<BYTE*>(_PsByteCode->GetBufferPointer()),
		_PsByteCode->GetBufferSize()
	};
}

UINT Shader::GetNumConstantBuffers()
{
	return _Reflection->GetBufferDesc().size();
}

UINT Shader::GetNumTextures()
{
	UINT count = 0;
	for (auto& bind : _Reflection->GetInputBinds())
		if (bind.Type == D3D_SIT_TEXTURE)
			++count;
	return count;
}

void Shader::PrintDebugInfo()
{
	LOG_TRACE("Shader reflection [{0}]", _Name);
	for (auto& cbuffer : _Reflection->GetBufferDesc())
	{
		const auto name = std::string(cbuffer.Name);
		LOG_TRACE(" {0}", name);

		auto it = _Reflection->GetCBufferVars().find(cbuffer.Name);
		if (it != _Reflection->GetCBufferVars().end())
		{
			for (auto& var : it->second)
			{
				LOG_TRACE("  {0} {1}", var.Type.Name, var.Name);
			}
		}
	}
}
