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

	void AddShader(const std::string& name);
	Shader* ShaderManager::GetShader(size_t index);
	Shader* GetShader(const std::string& name);

private:
	ShaderManager(){}

private:
	std::vector<std::unique_ptr<Shader>> _Shaders;
	std::unordered_map<std::string, size_t> _ShaderNameMap;
};
