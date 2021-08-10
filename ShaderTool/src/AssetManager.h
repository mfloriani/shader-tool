#pragma once

#include "Rendering/Mesh.h"
#include "Rendering/Texture.h"
#include "Rendering/Model.h"

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
	void SetTextureDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu, CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu);

	int AddMesh(std::shared_ptr<Mesh>& mesh);
	int LoadModelFromFile(const std::string& path);
	int AddModel(Model& model);
	Model GetModel(int index);
	
	int LoadTextureFromFile(const std::string& path);
	Texture* GetTexture(int index);

private:
	AssetManager() : _Device(nullptr), _CommandList(nullptr){}

private:
	ID3D12Device* _Device;
	ID3D12GraphicsCommandList* _CommandList;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _TexSrvCpuDescHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE _TexSrvGpuDescHandle;

	std::vector<std::shared_ptr<Mesh>> _Meshes;
	std::vector<Model> _Models;
	std::vector<std::unique_ptr<Texture>> _Textures;
	std::unordered_map<std::string, size_t> _MeshNameIndexMap;
	std::unordered_map<std::string, size_t> _ModelNameIndexMap;
	std::unordered_map<std::string, size_t> _TextureIndexMap;
};