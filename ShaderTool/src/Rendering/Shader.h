#pragma once

#include "D3DUtil.h"

class Shader
{
public:
	Shader(const std::string& name, Microsoft::WRL::ComPtr<ID3DBlob> buffer);
	~Shader();

	const std::string& GetName() const { return _Name; }
	D3D12_SHADER_BYTECODE GetByteCode();

	void PrintDebugInfo();
	
private:
	void Reflect();

private:
	Microsoft::WRL::ComPtr<ID3DBlob> _ByteCode;
	std::string _Name;
	D3DUtil::SHADER_DESC _ShaderDesc;
	std::vector<D3DUtil::SHADER_BUFFER_DESC> _CBuffersDesc;
	std::unordered_map<std::string, std::vector<D3DUtil::SHADER_VARIABLE_DESC>> _CBufferVars;

};