#include "pch.h"
#include "AssetManager.h"

AssetManager::~AssetManager()
{

}

bool AssetManager::HasMesh(const std::string& name)
{
	return _Meshes.find(name) != _Meshes.end();
}

void AssetManager::AddMesh(std::shared_ptr<Mesh>& mesh)
{
	if (mesh->Name.size() == 0)
	{
		LOG_WARN("Mesh has no name! Skipping...", mesh->Name);
		mesh->Name = "NO_NAME";
	}

	if (HasMesh(mesh->Name))
	{
		LOG_WARN("Mesh [{0}] ALREADY in AssetManager! Skipping...", mesh->Name);
		return;
	}
	_Meshes.insert({ mesh->Name, mesh });
}

std::shared_ptr<Mesh> AssetManager::GetMesh(const std::string& name)
{
	if (!HasMesh(name))
	{
		LOG_WARN("Mesh [{0}] NOT FOUND  in AssetManager", name);
		return std::make_shared<Mesh>();
	}
	return _Meshes[name];
}

bool AssetManager::HasModel(const std::string& name)
{
	return _Models.find(name) != _Models.end();
}

void AssetManager::AddModel(const std::string& name, Model& model)
{
	if (HasModel(name))
	{
		LOG_WARN("Model [{0}] ALREADY in AssetManager! Skipping...", name);
		return;
	}
	_Models.insert({ name, model });
}

Model AssetManager::GetModel(const std::string& name)
{
	if (!HasModel(name))
	{
		LOG_WARN("Model [{0}] NOT FOUND in AssetManager", name);
		return Model();
	}
	return _Models[name];
}
