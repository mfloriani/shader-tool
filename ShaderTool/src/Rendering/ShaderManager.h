#pragma once

#include "D3DUtil.h"
#include "Shader.h"

#include <string>
#include <vector>
#include <unordered_map>

enum class ShaderType
{
	Vertex,
	Pixel
};

class ShaderManager
{
public:
	static ShaderManager& Get()
	{
		static ShaderManager instance;
		return instance;
	}

	std::string LoadBinaryShader(const std::string& filename);
	std::string LoadRawShader(const std::string& filename, const std::string& entryPoint, const std::string& target);
	std::string LoadShaderFromFile(const std::string& filename, ShaderType type);

	Shader* GetShader(size_t index);
	Shader* GetShader(const std::string& name);
	const std::vector<std::unique_ptr<Shader>>& GetShaders() const { return _Shaders; }
	size_t GetShaderIndex(const std::string& name) { return _ShaderNameIndexMap[name]; }
	const std::string& GetShaderName(size_t index) { return _ShaderIndexNameMap[index]; }

private:
	ShaderManager(){}

private:
	std::vector<std::unique_ptr<Shader>> _Shaders;
	std::unordered_map<std::string, size_t> _ShaderNameIndexMap;
	std::unordered_map<size_t, std::string> _ShaderIndexNameMap;
};
