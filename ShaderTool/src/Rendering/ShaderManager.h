#pragma once

#include "D3DUtil.h"
#include "Shader.h"

#include <string>
#include <vector>
#include <unordered_map>

class ShaderManager
{
public:
	static ShaderManager& Get()
	{
		static ShaderManager instance;
		return instance;
	}

	int LoadBinaryShader(const std::string& name);
	int LoadRawShader(const std::string& filename, const std::string& entryPoint, const std::string& target);
	Shader* ShaderManager::GetShader(size_t index);
	Shader* GetShader(const std::string& name);

	const std::vector<std::unique_ptr<Shader>>& GetShaders() const { return _Shaders; }

private:
	ShaderManager(){}

private:
	std::vector<std::unique_ptr<Shader>> _Shaders;
	std::unordered_map<std::string, size_t> _ShaderNameMap;
};
