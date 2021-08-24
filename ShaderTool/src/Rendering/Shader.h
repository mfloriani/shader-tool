#pragma once

#include "ShaderReflection.h"

#include <map>

struct ShaderBind
{
	UINT RootParameterIndex;
	UINT BindPoint;
	D3D_SHADER_INPUT_TYPE BindType;
	std::string BindTypeName;
	std::string BindName;

	// Used for CBuffers
	std::string VarName{"NO_NAME"};
	std::string VarTypeName{"NO_TYPE_NAME"};
	UINT VarNum32BitValues{ 0 };
	UINT VarNum32BitValuesOffset{ 0 };

	// Used for Textures
	INT OffsetInDescriptors{ 0 };

	friend std::ostream& operator<<(std::ostream& out, const ShaderBind& rhs)
	{
		out << rhs.RootParameterIndex
			<< " " << rhs.BindPoint
			<< " " << rhs.BindType
			<< " " << rhs.BindTypeName
			<< " " << rhs.BindName
			<< " " << rhs.VarName
			<< " " << rhs.VarTypeName
			<< " " << rhs.VarNum32BitValues
			<< " " << rhs.VarNum32BitValuesOffset
			<< " " << rhs.OffsetInDescriptors;
		return out;
	}

	friend std::istream& operator>>(std::istream& in, ShaderBind& rhs)
	{
		int bindType;
		in >> rhs.RootParameterIndex
			>> rhs.BindPoint
			>> bindType
			>> rhs.BindTypeName
			>> rhs.BindName
			>> rhs.VarName
			>> rhs.VarTypeName
			>> rhs.VarNum32BitValues
			>> rhs.VarNum32BitValuesOffset
			>> rhs.OffsetInDescriptors;
		rhs.BindType = (D3D_SHADER_INPUT_TYPE)bindType;
		return in;
	}
};

struct RootParameter
{
	RootParameter(UINT index, D3D_SHADER_INPUT_TYPE type) : Index(index), Type(type) {}
	UINT Index;
	D3D_SHADER_INPUT_TYPE Type;
};

struct RootParameterConstants : public RootParameter
{
	RootParameterConstants(UINT index, D3D_SHADER_INPUT_TYPE type, UINT num32bitvalues, UINT bindpoint)
		: RootParameter(index, type), Num32BitValues(num32bitvalues), BindPoint(bindpoint) {}

	UINT Num32BitValues;
	UINT BindPoint;
};

struct RootParameterDescriptorTable : public RootParameter
{
	RootParameterDescriptorTable(
		UINT index, 
		D3D_SHADER_INPUT_TYPE type,
		D3D12_DESCRIPTOR_RANGE_TYPE descRangeType, 
		UINT numDescriptors, 
		UINT bindpoint, 
		D3D12_SHADER_VISIBILITY shaderVisibility)
		: RootParameter(index, type),
		DescRangeType(descRangeType), 
		NumDescriptors(numDescriptors),
		BindPoint(bindpoint),
		ShaderVisibility(shaderVisibility)
	{}

	D3D12_DESCRIPTOR_RANGE_TYPE DescRangeType;
	UINT NumDescriptors;
	UINT BindPoint;
	D3D12_SHADER_VISIBILITY ShaderVisibility;
};

class Shader
{
public:
	Shader(const std::string& name, Microsoft::WRL::ComPtr<ID3DBlob> vsBuffer, Microsoft::WRL::ComPtr<ID3DBlob> psBuffer);
	~Shader();

	const std::string& GetName() const { return _Name; }
	D3D12_SHADER_BYTECODE GetVsByteCode();
	D3D12_SHADER_BYTECODE GetPsByteCode();

	UINT GetNumConstantBuffers();
	UINT GetNumTextures();
	UINT GetNumSamplers();

	const std::vector<ShaderBind>& GetBindingVars() const { return _BindingVarsMap; }
	const std::vector<std::unique_ptr<RootParameter>>& GetRootParameters() const { return _RootParameters; }
	
private:
	void BuildRootParameters();

private:
	std::string                                 _Name;
	Microsoft::WRL::ComPtr<ID3DBlob>            _VsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob>            _PsByteCode;
	std::unique_ptr<ShaderReflection>           _Reflection;	
	std::vector<std::unique_ptr<RootParameter>> _RootParameters;
	std::vector<ShaderBind>                     _BindingVarsMap;
};