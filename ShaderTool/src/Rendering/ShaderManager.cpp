#include "pch.h"
#include "ShaderManager.h"

using namespace DirectX;
using namespace D3DUtil;
using Microsoft::WRL::ComPtr;

void ShaderManager::AddShader(const std::string& name)
{
	LOG_TRACE("Loading shader {0}", name);
	ComPtr<ID3DBlob> blob;
	const std::string filename = name + ".cso";
	ThrowIfFailed(D3DReadFileToBlob(AnsiToWString(filename).c_str(), &blob));

	_Shaders.push_back(std::make_unique<Shader>(name, blob));
	_ShaderNameMap.insert(std::make_pair(name, _Shaders.size() - 1));
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
