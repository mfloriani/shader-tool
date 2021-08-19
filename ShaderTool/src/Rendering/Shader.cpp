#include "pch.h"
#include "Shader.h"
#include "Editor/Graph/Node.h"

using Microsoft::WRL::ComPtr;
using namespace D3DUtil;

Shader::Shader(const std::string& name, ComPtr<ID3DBlob> vsBuffer, ComPtr<ID3DBlob> psBuffer)
	: _Name(name), _VsByteCode(vsBuffer), _PsByteCode(psBuffer)
{
	LOG_TRACE("Shader Reflection {0}", _Name);
	_Reflection = std::make_unique<ShaderReflection>(vsBuffer, psBuffer);
	BuildRootParameters();
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
	return (UINT)_Reflection->GetBufferDesc().size();
}

UINT Shader::GetNumTextures()
{
	UINT count = 0;
	for (auto& bind : _Reflection->GetInputBinds())
		if (bind.Type == D3D_SIT_TEXTURE)
			++count;
	return count;
}

UINT Shader::GetNumSamplers()
{
	UINT count = 0;
	for (auto& bind : _Reflection->GetInputBinds())
		if (bind.Type == D3D_SIT_SAMPLER)
			++count;
	return count;
}

void Shader::BuildRootParameters()
{
	auto& cbufferVars = _Reflection->GetCBufferVars();
	auto& bindings = _Reflection->GetInputBinds();
	
	UINT numBindings = (UINT)bindings.size();
	UINT numSamplers = GetNumSamplers();

	numBindings -= numSamplers; // ignoring samplers for now

	_RootParameters.clear();
	//_RootParameters.resize(numBindings, nullptr);
	UINT rootParameterIndex = 0;

	for (auto& bind : bindings)
	{
		LOG_TRACE("BIND {0} {1} {2}", bind.BindPoint, bind.Type, bind.Name);
		UINT num32BitValuesOffset = 0;
		UINT num32BitValuesTotal = 0;
		
		if (bind.Type == D3D_SIT_SAMPLER) // TODO: ignoring samplers for now
		{
			LOG_TRACE("Skipping D3D_SIT_SAMPLER...");
			continue;
		}

		switch (bind.Type)
		{
		case D3D_SIT_CBUFFER:
		{
			auto it = cbufferVars.find(bind.Name);
			if (it != cbufferVars.end())
			{
				for (auto var : it->second)
				{
					UINT num32BitValues = HlslNodeType::Get()->GetNum32BitValues(var.Type.Name);
					if (num32BitValues == 0)
					{
						LOG_CRITICAL("DirectXMath value for {0} is not mapped", var.Type.Name);
						throw std::runtime_error("DirectXMath value for "+ var.Type.Name +" is not mapped");
					}
					LOG_TRACE("  var {0} {1} {2}", var.Name, var.Type.Name, num32BitValues);

					ShaderBind shaderBind;
					shaderBind.RootParameterIndex = rootParameterIndex;
					shaderBind.BindPoint = bind.BindPoint;
					shaderBind.BindType = bind.Type;
					shaderBind.BindTypeName = magic_enum::enum_name(bind.Type);
					shaderBind.BindName = bind.Name;

					shaderBind.VarName = var.Name;
					shaderBind.VarTypeName = var.Type.Name;
					shaderBind.VarNum32BitValues = num32BitValues;
					shaderBind.VarNum32BitValuesOffset = num32BitValuesOffset;

					_BindingVarsMap.push_back(shaderBind);

					num32BitValuesOffset += num32BitValues;
					num32BitValuesTotal += num32BitValues;
				}
			}
			else
			{
				LOG_WARN("Variables NOT FOUND for Constant Buffer {0} ", bind.Name);
			}

			LOG_TRACE("RootParameters[{0}].InitAsConstants({1}, {2})", rootParameterIndex, num32BitValuesTotal, bind.BindPoint);

			_RootParameters.push_back(
				std::make_unique<RootParameterConstants>(
					rootParameterIndex, 
					bind.Type,
					num32BitValuesTotal, 
					bind.BindPoint));
		}
		break;

		case D3D_SIT_TEXTURE:
		{
			ShaderBind shaderBind{};
			shaderBind.RootParameterIndex = rootParameterIndex;
			shaderBind.BindPoint = bind.BindPoint;
			shaderBind.BindType = bind.Type;
			shaderBind.BindTypeName = magic_enum::enum_name(bind.Type);
			shaderBind.BindName = bind.Name;
			shaderBind.OffsetInDescriptors = 2; // TODO: should not be fixed 
			
			_BindingVarsMap.push_back(shaderBind);

			LOG_TRACE("RootParameters[{0}].InitAsDescriptorTable {1}", rootParameterIndex, bind.BindPoint);
			
			auto rootPardescTable = std::make_unique<RootParameterDescriptorTable>(
				rootParameterIndex, 
				bind.Type,
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
				1, 
				bind.BindPoint, 
				D3D12_SHADER_VISIBILITY_PIXEL);

			_RootParameters.push_back( std::move(rootPardescTable) );
		}
		break;

		//case D3D_SIT_SAMPLER:
		//{
		//	LOG_TRACE("Ignoring D3D_SIT_SAMPLER");
		//	//CD3DX12_DESCRIPTOR_RANGE texTable;
		//	//texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, bind.BindPoint);
		//	//_RootParameters[rootParameterIndex].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		//}
		//break;

		default:
			break;
		}

		++rootParameterIndex;
	}
}
