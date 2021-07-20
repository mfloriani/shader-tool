#pragma once

#include "D3DUtil.h"

class Shader
{
public:
	Shader(const std::string& name, Microsoft::WRL::ComPtr<ID3DBlob> buffer);
	~Shader();

	const std::string& GetName() const { return _Name; }
	D3D12_SHADER_BYTECODE GetByteCode();
	
private:
	void Reflect();

private:
	Microsoft::WRL::ComPtr<ID3DBlob> _BufferData;
	std::string _Name;

	struct CBUFFER_VARIABLE
	{
		CBUFFER_VARIABLE(D3D12_SHADER_VARIABLE_DESC varDesc, D3D12_SHADER_TYPE_DESC varType)
			: Desc(varDesc), Type(varType) {}

		D3D12_SHADER_VARIABLE_DESC Desc;
		D3D12_SHADER_TYPE_DESC     Type;
	};

	D3D12_SHADER_DESC _ShaderDesc;
	std::vector<D3D12_SHADER_BUFFER_DESC> _CBuffersDesc;
	std::unordered_map<std::string, CBUFFER_VARIABLE> _CBufferVars;

};