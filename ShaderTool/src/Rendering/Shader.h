#pragma once

#include "ShaderReflection.h"

#include <map>

struct ShaderBind
{
	UINT BindPoint;
	D3D_SHADER_INPUT_TYPE BindType;
	std::string BindTypeName;
	std::string BindName;

	std::string VarName;
	std::string VarTypeName;
	UINT VarNum32BitValues;
	UINT VarNum32BitValuesOffset;
};

class Shader
{
public:
	using RootParameterId = UINT;
	using BindingVarsMap = std::map<RootParameterId, std::vector<ShaderBind>>;

public:
	Shader(const std::string& name, Microsoft::WRL::ComPtr<ID3DBlob> vsBuffer, Microsoft::WRL::ComPtr<ID3DBlob> psBuffer);
	~Shader();

	const std::string& GetName() const { return _Name; }
	D3D12_SHADER_BYTECODE GetVsByteCode();
	D3D12_SHADER_BYTECODE GetPsByteCode();

	UINT GetNumConstantBuffers();
	UINT GetNumTextures();

	const BindingVarsMap& GetBindingVars() const { return _BindingVarsMap; }
	const std::vector<CD3DX12_ROOT_PARAMETER>& GetRootParameters() const { return _RootParameters; }
	
private:
	void BuildRootParameters();

private:
	std::string                           _Name;
	Microsoft::WRL::ComPtr<ID3DBlob>      _VsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob>      _PsByteCode;
	std::unique_ptr<ShaderReflection>     _Reflection;
	std::vector<CD3DX12_ROOT_PARAMETER>   _RootParameters;
	BindingVarsMap                        _BindingVarsMap;
};