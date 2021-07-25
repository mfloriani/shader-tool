#include "pch.h"
#include "ShaderManager.h"

using namespace DirectX;
using namespace D3DUtil;
using Microsoft::WRL::ComPtr;

std::string ShaderManager::LoadBinaryShader(const std::string& filename)
{
	LOG_TRACE("Loading cso shader {0}", filename);
	ComPtr<ID3DBlob> blob;
	const std::string name = D3DUtil::ExtractFilename(filename);
	ThrowIfFailed(D3DReadFileToBlob(AnsiToWString(filename).c_str(), &blob));

	_Shaders.push_back(std::make_unique<Shader>(name, blob));
	_ShaderNameIndexMap.insert(std::make_pair(name, _Shaders.size() - 1u));
	_ShaderIndexNameMap.insert(std::make_pair(_Shaders.size() - 1u, name));

	return name;
}

std::string ShaderManager::LoadRawShader(const std::string& filename, const std::string& entryPoint, const std::string& target)
{
	LOG_TRACE("Loading hlsl shader [{0} | {1} | {2}]", filename, entryPoint, target);
	ComPtr<ID3DBlob> blob;
	if (D3DUtil::CompileShader(filename, entryPoint, target, blob))
	{
		const auto name = D3DUtil::ExtractFilename(filename);	
		_Shaders.push_back(std::make_unique<Shader>(name, blob));
		_ShaderNameIndexMap.insert(std::make_pair(name, _Shaders.size() - 1u));
		_ShaderIndexNameMap.insert(std::make_pair(_Shaders.size() - 1u, name));

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
