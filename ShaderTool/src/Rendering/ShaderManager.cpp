#include "pch.h"
#include "ShaderManager.h"

using namespace DirectX;
using namespace D3DUtil;
using Microsoft::WRL::ComPtr;

ShaderManager* ShaderManager::_Instance = nullptr;

void ShaderManager::LoadBinaryShader(const std::string& path, ComPtr<ID3DBlob>& bytecode)
{
	LOG_TRACE("Loading cso shader {0}", path);
	const std::string name = D3DUtil::ExtractFilename(path);
	ThrowIfFailed(D3DReadFileToBlob(AnsiToWString(path).c_str(), &bytecode));
}

std::ostream& ShaderManager::Serialize(std::ostream& out)
{
	auto this_ = ShaderManager::Get();
	auto& shaders = this_->GetShaders();
	out << shaders.size() << "\n";
	for (size_t i = 0; i < shaders.size(); ++i)
	{
		const std::string& name = this_->_ShaderIndexNameMap.find(i)->second;
		const std::string& path = this_->_ShaderIndexPathMap.find(i)->second;
		out << i << " " << name << " " << path << "\n";
	}
	return out;
}

std::istream& ShaderManager::Deserialize(std::istream& in)
{
	size_t numShaders;
	in >> numShaders;

	size_t index;
	std::string name, path;

	auto this_ = ShaderManager::Get();
	for (size_t i = 0; i < numShaders; ++i)
	{
		in >> index >> name >> path;
		this_->LoadShaderFromFile(path);
	}
	return in;
}

std::string ShaderManager::LoadShaderFromFile(const std::string& path)
{
	std::string name = D3DUtil::ExtractFilename(path);
	
	bool vsOk = false;
	ComPtr<ID3DBlob> vsBlob;
	{
		std::string entryPoint = "VS";
		std::string target = "vs_5_0";
		LOG_TRACE("Compiling shader [{0} | {1} | {2}]", path, entryPoint, target);
		if (D3DUtil::CompileShader(path, entryPoint, target, vsBlob))
			vsOk = true;
	}

	bool psOk = false;
	ComPtr<ID3DBlob> psBlob;
	{
		std::string entryPoint = "PS";
		std::string target = "ps_5_0";
		LOG_TRACE("Compiling shader [{0} | {1} | {2}]", path, entryPoint, target);
		if (D3DUtil::CompileShader(path, entryPoint, target, psBlob))
			psOk = true;
	}

	if (vsOk && psOk)
	{
		if (!HasShader(name)) // new shader
		{
			_Shaders.push_back(std::make_unique<Shader>(name, vsBlob, psBlob));
			size_t index = _Shaders.size() - 1u;
			_ShaderNameIndexMap.insert(std::make_pair(name, index));
			_ShaderIndexNameMap.insert(std::make_pair(index, name));
			_ShaderIndexPathMap.insert(std::make_pair(index, path));
		}
		else // overwrite the current shader
		{
			size_t index = GetShaderIndex(name);
			_Shaders[index] = std::make_unique<Shader>(name, vsBlob, psBlob);
			_ShaderIndexPathMap.insert(std::make_pair(index, path));
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
