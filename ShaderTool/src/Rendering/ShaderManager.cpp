#include "pch.h"
#include "ShaderManager.h"

using namespace DirectX;
using namespace D3DUtil;
using Microsoft::WRL::ComPtr;

void ShaderManager::LoadBinaryShader(const std::string& filename, ComPtr<ID3DBlob>& bytecode)
{
	LOG_TRACE("Loading cso shader {0}", filename);
	const std::string name = D3DUtil::ExtractFilename(filename);
	ThrowIfFailed(D3DReadFileToBlob(AnsiToWString(filename).c_str(), &bytecode));
}

std::string ShaderManager::LoadShaderFromFile(const std::string& filename)
{
	std::string name = D3DUtil::ExtractFilename(filename);
	
	bool vsOk = false;
	ComPtr<ID3DBlob> vsBlob;
	{
		std::string entryPoint = "VS";
		std::string target = "vs_5_0";
		LOG_TRACE("Compiling shader [{0} | {1} | {2}]", filename, entryPoint, target);		
		if (D3DUtil::CompileShader(filename, entryPoint, target, vsBlob))
			vsOk = true;
	}

	bool psOk = false;
	ComPtr<ID3DBlob> psBlob;
	{
		std::string entryPoint = "PS";
		std::string target = "ps_5_0";
		LOG_TRACE("Compiling shader [{0} | {1} | {2}]", filename, entryPoint, target);
		if (D3DUtil::CompileShader(filename, entryPoint, target, psBlob))
			psOk = true;
	}

	if (vsOk && psOk)
	{
		if (!HasShader(name)) // new shader
		{
			_Shaders.push_back(std::make_unique<Shader>(name, vsBlob, psBlob));
			_ShaderNameIndexMap.insert(std::make_pair(name, _Shaders.size() - 1u));
			_ShaderIndexNameMap.insert(std::make_pair(_Shaders.size() - 1u, name));
		}
		else // overwrite the current shader
		{
			size_t index = GetShaderIndex(name);
			_Shaders[index] = std::make_unique<Shader>(name, vsBlob, psBlob);
		}
		return name;
	}

	return std::string();
}

Shader* ShaderManager::GetShader(size_t index)
{
	if (index > _Shaders.size() - 1) // Index bigger than vector size
	{
		LOG_ERROR("INVALID Shader index {0}", index);
		return nullptr;
	}
	return _Shaders[index].get();
}

Shader* ShaderManager::GetShader(const std::string& name)
{
	auto it = _ShaderNameIndexMap.find(name);
	if (it == _ShaderNameIndexMap.end()) // NOT FOUND
	{
		LOG_ERROR("Shader {0} NOT LOADED", name);
		return nullptr;
	}
	return GetShader(it->second);
}
