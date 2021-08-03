#pragma once

#include "D3DUtil.h"

#include <vector>
#include <unordered_map>

class ShaderReflection
{
public:
	using ShaderVec = std::vector<D3DUtil::SHADER_DESC>;
	using CBufferVec = std::vector<D3DUtil::SHADER_BUFFER_DESC>;
	using CBufferVarMap = std::unordered_map<std::string, std::vector<D3DUtil::SHADER_VARIABLE_DESC>>; // cbuffer name x var desc
	using InputBindVec = std::vector<D3DUtil::SHADER_INPUT_BIND_DESC>;
	using SignatureParameterVec = std::vector<D3DUtil::SIGNATURE_PARAMETER_DESC>;

public:
	ShaderReflection(Microsoft::WRL::ComPtr<ID3DBlob>& vsBytecode, Microsoft::WRL::ComPtr<ID3DBlob>& psBytecode);
	
	const ShaderVec& GetShadersDesc() const { return _ShadersDesc; }
	const CBufferVec& GetBufferDesc() const { return _CBuffersDesc; }
	const CBufferVarMap& GetCBufferVars() const { return _CBufferVars; }
	const InputBindVec& GetInputBinds() const { return _ShaderInputBinds; }
	const SignatureParameterVec& GetInputs() const { return _ShaderInputs; }
	const SignatureParameterVec& GetOutputs() const { return _ShaderOutputs; }
	const UINT GetNumTotalCBuffervars() const { return _NumTotalCBufferVars; }

private:
	void Reflect(Microsoft::WRL::ComPtr<ID3DBlob>& bytecode);

private:
	ShaderVec _ShadersDesc;
	CBufferVec _CBuffersDesc;
	CBufferVarMap _CBufferVars;
	InputBindVec _ShaderInputBinds;
	SignatureParameterVec _ShaderInputs;
	SignatureParameterVec _ShaderOutputs;
	UINT _NumTotalCBufferVars;
};