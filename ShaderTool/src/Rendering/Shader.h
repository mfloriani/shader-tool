#pragma once

#include "ShaderReflection.h"

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

	void PrintDebugInfo();
	
private:
	std::string _Name;
	Microsoft::WRL::ComPtr<ID3DBlob>  _VsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob>  _PsByteCode;
	std::unique_ptr<ShaderReflection> _Reflection;

};