#include "pch.h"
#include "Shader.h"

using Microsoft::WRL::ComPtr;

Shader::Shader(const std::string& name, ComPtr<ID3DBlob> buffer) 
	: _Name(name), _BufferData(buffer)
{
	Reflect();
}

Shader::~Shader()
{
}

D3D12_SHADER_BYTECODE Shader::GetByteCode()
{
	return { 
		reinterpret_cast<BYTE*>(_BufferData->GetBufferPointer()), 
		_BufferData->GetBufferSize()	
	};
}

void Shader::Reflect()
{
	ID3D12ShaderReflection* shaderReflection;
	auto hr = D3DReflect(
		_BufferData->GetBufferPointer(),
		_BufferData->GetBufferSize(),
		IID_ID3D12ShaderReflection,
		reinterpret_cast<void**>(&shaderReflection)
	);
	shaderReflection->GetDesc(&_ShaderDesc);
	
	for (unsigned int i = 0; i < _ShaderDesc.ConstantBuffers; ++i)
	{
		ID3D12ShaderReflectionConstantBuffer* buffer = shaderReflection->GetConstantBufferByIndex(i);

		D3D12_SHADER_BUFFER_DESC bufferDesc;
		buffer->GetDesc(&bufferDesc);
		LOG_TRACE("CBuffer {0}", bufferDesc.Name);
		_CBuffersDesc.push_back(bufferDesc);

		for (UINT j = 0; j < bufferDesc.Variables; j++)
		{
			ID3D12ShaderReflectionVariable* var = buffer->GetVariableByIndex(j);
			D3D12_SHADER_VARIABLE_DESC varDesc;
			var->GetDesc(&varDesc);

			auto type = var->GetType();
			D3D12_SHADER_TYPE_DESC typeDesc;
			type->GetDesc(&typeDesc);

			LOG_TRACE("  {0} {1}", typeDesc.Name, varDesc.Name);

			_CBufferVars.insert(
				std::make_pair(
					bufferDesc.Name, 
					CBUFFER_VARIABLE(varDesc, typeDesc)));
		}
	}

	for (unsigned int i = 0; i < _ShaderDesc.BoundResources; ++i)
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc = {};
		shaderReflection->GetResourceBindingDesc(i, &bindDesc);

		LOG_TRACE("INPUT_BIND_DESC {0} {1}", bindDesc.Type, bindDesc.Name);
	}


	shaderReflection->Release();
}
