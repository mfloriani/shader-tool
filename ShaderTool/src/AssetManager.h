#pragma once

#include "Rendering\Mesh.h"
#include "Model.h"

class AssetManager
{
public:
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	~AssetManager();

	static AssetManager& Get()
	{
		static AssetManager instance;
		return instance;
	}

	bool HasMesh(const std::string& name);
	void AddMesh(std::shared_ptr<Mesh>& mesh);
	std::shared_ptr<Mesh> GetMesh(const std::string& name);

	bool HasModel(const std::string& name);
	void AddModel(const std::string& name, Model& model);
	Model GetModel(const std::string& name);

private:
	AssetManager() {}

private:
	std::unordered_map<std::string, std::shared_ptr<Mesh>>  _Meshes;
	std::unordered_map<std::string, Model> _Models;
};