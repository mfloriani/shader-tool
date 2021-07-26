#include "pch.h"
#include "Shader.h"

#include <bitset>

using Microsoft::WRL::ComPtr;
using namespace D3DUtil;

Shader::Shader(const std::string& name, ComPtr<ID3DBlob> buffer) 
	: _Name(name), _ByteCode(buffer)
{
	Reflect();
}

Shader::~Shader()
{
}

D3D12_SHADER_BYTECODE Shader::GetByteCode()
{
	return { 
		reinterpret_cast<BYTE*>(_ByteCode->GetBufferPointer()), 
		_ByteCode->GetBufferSize()	
	};
}


void Shader::Reflect()
{
	ID3D12ShaderReflection* shaderReflection;
	auto hr = D3DReflect(
		_ByteCode->GetBufferPointer(),
		_ByteCode->GetBufferSize(),
		IID_ID3D12ShaderReflection,
		reinterpret_cast<void**>(&shaderReflection)
	);
	D3D12_SHADER_DESC shaderDesc;
	shaderReflection->GetDesc(&shaderDesc);
	_ShaderDesc = SHADER_DESC(shaderDesc);

	// CONSTANT BUFFERS
	for (unsigned int i = 0; i < _ShaderDesc.ConstantBuffers; ++i)
	{
		ID3D12ShaderReflectionConstantBuffer* buffer = shaderReflection->GetConstantBufferByIndex(i);

		D3D12_SHADER_BUFFER_DESC bufferDesc;
		buffer->GetDesc(&bufferDesc);
		LOG_TRACE("CBUFFER {0}", bufferDesc.Name);
		_CBuffersDesc.push_back(SHADER_BUFFER_DESC(bufferDesc));

		_CBufferVars.insert(
			std::make_pair( 
				bufferDesc.Name, 
				std::vector<SHADER_VARIABLE_DESC>()));

		_CBufferVars[bufferDesc.Name].reserve(bufferDesc.Variables);

		// CONSTANT BUFFER VARS
		for (UINT j = 0; j < bufferDesc.Variables; j++)
		{
			ID3D12ShaderReflectionVariable* var = buffer->GetVariableByIndex(j);
			D3D12_SHADER_VARIABLE_DESC varDesc;
			var->GetDesc(&varDesc);

			auto type = var->GetType();
			D3D12_SHADER_TYPE_DESC typeDesc;
			type->GetDesc(&typeDesc);

			LOG_TRACE("  {0} {1}", typeDesc.Name, varDesc.Name);

			_CBufferVars[bufferDesc.Name].push_back(SHADER_VARIABLE_DESC(varDesc, typeDesc));
		}
	}

	// TEXTURES and SAMPLERS
	for (unsigned int i = 0; i < _ShaderDesc.BoundResources; ++i)
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc = {};
		shaderReflection->GetResourceBindingDesc(i, &bindDesc);

		LOG_TRACE("INPUT_BIND_DESC {0} {1}", bindDesc.Type, bindDesc.Name);
	}

	// SHADER INPUT
	for (unsigned int i = 0; i < _ShaderDesc.InputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC desc;
		shaderReflection->GetInputParameterDesc(i, &desc);

		LOG_TRACE("INPUT {0} | {1} | {2} -> {3}, {4}, {5} ", 
			desc.Register, desc.ComponentType, desc.SemanticIndex, desc.SemanticName, 
			std::bitset<8>(desc.Mask), std::bitset<8>(desc.ReadWriteMask));
	}

	// SHADER OUTPUT
	for (unsigned int i = 0; i < _ShaderDesc.OutputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC desc;
		shaderReflection->GetOutputParameterDesc(i, &desc);

		LOG_TRACE("OUTPUT {0} | {1} | {2} -> {3}, {4}, {5} ",
			desc.Register, desc.ComponentType, desc.SemanticIndex, desc.SemanticName,
			std::bitset<8>(desc.Mask), std::bitset<8>(desc.ReadWriteMask));
	}


	shaderReflection->Release();
}

void Shader::PrintDebugInfo()
{
	LOG_TRACE("[{0}]", _Name);
	for (auto& cbuffer : _CBuffersDesc)
	{
		const auto name = std::string (cbuffer.Name);
		LOG_TRACE(" {0}", name);

		auto it = _CBufferVars.find(cbuffer.Name);
		if (it != _CBufferVars.end())
		{
			for (auto& var : it->second)
			{
				LOG_TRACE("  {0} {1}", var.Type.Name, var.Name);
			}
		}
	}
}
