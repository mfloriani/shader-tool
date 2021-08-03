#include "pch.h"
#include "ShaderReflection.h"

#include <bitset>

using Microsoft::WRL::ComPtr;
using namespace D3DUtil;

ShaderReflection::ShaderReflection(Microsoft::WRL::ComPtr<ID3DBlob>& vsBytecode, Microsoft::WRL::ComPtr<ID3DBlob>& psBytecode) : _NumTotalCBufferVars(0)
{
	Reflect(vsBytecode);
	Reflect(psBytecode);
}

void ShaderReflection::Reflect(Microsoft::WRL::ComPtr<ID3DBlob>& bytecode)
{
	ID3D12ShaderReflection* shaderReflection;
	auto hr = D3DReflect(
		bytecode->GetBufferPointer(),
		bytecode->GetBufferSize(),
		IID_ID3D12ShaderReflection,
		reinterpret_cast<void**>(&shaderReflection)
	);
	D3D12_SHADER_DESC shaderDesc;
	shaderReflection->GetDesc(&shaderDesc);
	_ShadersDesc.push_back( SHADER_DESC(shaderDesc) );

	auto& shdr = _ShadersDesc[_ShadersDesc.size() - 1];

	// CONSTANT BUFFERS
	for (unsigned int i = 0; i < shdr.ConstantBuffers; ++i)
	{
		ID3D12ShaderReflectionConstantBuffer* buffer = shaderReflection->GetConstantBufferByIndex(i);

		D3D12_SHADER_BUFFER_DESC bufferDesc;
		buffer->GetDesc(&bufferDesc);
		LOG_TRACE("CBUFFER {0}", bufferDesc.Name);
		_CBuffersDesc.push_back(SHADER_BUFFER_DESC(bufferDesc));
		
		_CBufferVars[bufferDesc.Name] = std::vector<SHADER_VARIABLE_DESC>();
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
			++_NumTotalCBufferVars;
		}
	}

	// CONSTANT BUFFERS, TEXTURES and SAMPLERS
	for (unsigned int i = 0; i < shdr.BoundResources; ++i)
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc = {};
		shaderReflection->GetResourceBindingDesc(i, &bindDesc);

		LOG_TRACE("INPUT_BIND_DESC {0} {1} {2} {3}", bindDesc.BindPoint, bindDesc.Type, magic_enum::enum_name(bindDesc.Type), bindDesc.Name);

		_ShaderInputBinds.push_back(SHADER_INPUT_BIND_DESC(bindDesc));
	}

	// SHADER INPUT
	for (unsigned int i = 0; i < shdr.InputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC desc;
		shaderReflection->GetInputParameterDesc(i, &desc);

		LOG_TRACE("INPUT  {0} | {1} | {2} | {3} | {4} -> {5}",
			desc.Register, desc.ComponentType, std::bitset<8>(desc.Mask), std::bitset<8>(desc.ReadWriteMask), desc.SemanticIndex, desc.SemanticName);

		_ShaderInputs.push_back(desc);
	}

	// SHADER OUTPUT
	for (unsigned int i = 0; i < shdr.OutputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC desc;
		shaderReflection->GetOutputParameterDesc(i, &desc);

		LOG_TRACE("OUTPUT {0} | {1} | {2} | {3} | {4} -> {5}",
			desc.Register, desc.ComponentType, std::bitset<8>(desc.Mask), std::bitset<8>(desc.ReadWriteMask), desc.SemanticIndex, desc.SemanticName);

		_ShaderOutputs.push_back(desc);
	}

	shaderReflection->Release();
}
