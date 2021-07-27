#pragma once

#include "D3DUtil.h"

class Shader
{
public:
	Shader(const std::string& name, Microsoft::WRL::ComPtr<ID3DBlob> vsBuffer, Microsoft::WRL::ComPtr<ID3DBlob> psBuffer);
	~Shader();

	const std::string& GetName() const { return _Name; }
	D3D12_SHADER_BYTECODE GetVsByteCode();
	D3D12_SHADER_BYTECODE GetPsByteCode();

	void PrintDebugInfo();
	
private:
	void Reflect(Microsoft::WRL::ComPtr<ID3DBlob>& bytecode);

private:
	Microsoft::WRL::ComPtr<ID3DBlob> _VsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> _PsByteCode;
	std::string _Name;
	D3DUtil::SHADER_DESC _ShaderDesc;
	std::vector<D3DUtil::SHADER_BUFFER_DESC> _CBuffersDesc;
	std::unordered_map<std::string, std::vector<D3DUtil::SHADER_VARIABLE_DESC>> _CBufferVars;

};