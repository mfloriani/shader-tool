#include "pch.h"
#include "ShaderManager.h"

using namespace DirectX;
using namespace D3DUtil;
using Microsoft::WRL::ComPtr;

bool ShaderManager::LoadBinaryShader(const std::string& name)
{
	LOG_TRACE("Loading cso shader {0}", name);
	ComPtr<ID3DBlob> blob;
	const std::string filename = name + ".cso";
	ThrowIfFailed(D3DReadFileToBlob(AnsiToWString(filename).c_str(), &blob));

	_Shaders.push_back(std::make_unique<Shader>(name, blob));
	_ShaderNameMap.insert(std::make_pair(name, _Shaders.size() - 1));
	
	return true;
}

bool ShaderManager::LoadRawShader(const std::string& filename, const std::string& entryPoint, const std::string& target)
{
	LOG_TRACE("Loading hlsl shader [{0} | {1} | {2}]", filename, entryPoint, target);
	ComPtr<ID3DBlob> blob;
	if (D3DUtil::CompileShader(filename, entryPoint, target, blob))
	{
		auto name = D3DUtil::ExtractFilename(filename);	
		_Shaders.push_back(std::make_unique<Shader>(name, blob));
		_ShaderNameMap.insert(std::make_pair(name, _Shaders.size() - 1));
		return true;
	}
	return false;
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
	auto it = _ShaderNameMap.find(name);
	if (it == _ShaderNameMap.end()) // NOT FOUND
	{
		LOG_ERROR("Shader {0} NOT LOADED", name);
		return nullptr;
	}
	return GetShader(it->second);
}
