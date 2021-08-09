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

	bool Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

	int LoadModelFromFile(const std::string& path);

	int AddMesh(std::shared_ptr<Mesh>& mesh);
	int AddModel(Model& model);

	//bool HasMesh(const std::string& name);
	//bool HasModel(const std::string& name);

	//std::shared_ptr<Mesh> GetMesh(const std::string& name);
	//size_t GetMeshIndex(const std::string& name);
	
	Model GetModel(int index);
	//size_t GetModelIndex(const std::string& name);
	
private:
	AssetManager() : _Device(nullptr), _CommandList(nullptr){}

private:
	ID3D12Device* _Device;
	ID3D12GraphicsCommandList* _CommandList;

	std::vector<std::shared_ptr<Mesh>> _Meshes;
	std::vector<Model> _Models;

	using NameIndexMap = std::unordered_map<std::string, size_t>;
	NameIndexMap _MeshNameIndexMap;
	NameIndexMap _ModelNameIndexMap;
};