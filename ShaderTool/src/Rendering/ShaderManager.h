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
	static ShaderManager* Get()
	{
		if (!_Instance)
			_Instance = new ShaderManager();

		return _Instance;
	}

	void LoadBinaryShader(const std::string& path, Microsoft::WRL::ComPtr<ID3DBlob>& bytecode);
	std::string LoadShaderFromFile(const std::string& path);

	Shader* GetShader(size_t index);
	Shader* GetShader(const std::string& name);
	std::vector<std::unique_ptr<Shader>>& GetShaders() { return _Shaders; }
	size_t GetShaderIndex(const std::string& name) { return _ShaderNameIndexMap[name]; }
	const std::string& GetShaderName(size_t index) { return _ShaderIndexNameMap[index]; }
	bool HasShader(const std::string& name) { return _ShaderNameIndexMap.find(name) != _ShaderNameIndexMap.end(); }

	std::ostream& Serialize(std::ostream& out);
	std::istream& Deserialize(std::istream& in);

private:
	static ShaderManager* _Instance;
	ShaderManager(){}

private:
	std::vector<std::unique_ptr<Shader>>    _Shaders;
	std::unordered_map<std::string, size_t> _ShaderNameIndexMap;
	std::unordered_map<size_t, std::string> _ShaderIndexNameMap;
	std::unordered_map<size_t, std::string> _ShaderIndexPathMap;
};
