#include "pch.h"
#include "AssetManager.h"
#include "Rendering/Vertex.h"
#include "Defines.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

AssetManager::~AssetManager()
{

}

// MESH

//bool AssetManager::HasMesh(const std::string& name)
//{
//	return _MeshNameIndexMap[type].find(name) != _MeshNameIndexMap[type].end();
//}

bool AssetManager::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	_Device = device;
	_CommandList = cmdList;

	return true;
}

int AssetManager::LoadModelFromFile(const std::string& path)
{
	std::string name = D3DUtil::ExtractFilename(path);

	Assimp::Importer importer;
	const aiScene* const scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_GenUVCoords);
	aiMesh* const importedMesh = scene->mMeshes[0];

	auto mesh = std::make_shared<Mesh>();
	mesh->Name = name;

	std::vector<Vertex> vertices;
	vertices.reserve(importedMesh->mNumVertices);

	for (uint32_t i = 0; i < importedMesh->mNumVertices; ++i)
	{
		const float px = importedMesh->mVertices[i].x;
		const float py = importedMesh->mVertices[i].y;
		const float pz = importedMesh->mVertices[i].z;

		const float nx = importedMesh->mNormals[i].x;
		const float ny = importedMesh->mNormals[i].y;
		const float nz = importedMesh->mNormals[i].z;

		const float tx = importedMesh->mTangents[i].x;
		const float ty = importedMesh->mTangents[i].y;
		const float tz = importedMesh->mTangents[i].z;

		float u = 0.f;
		float v = 0.f;
		if (importedMesh->mTextureCoords[0])
		{
			u = importedMesh->mTextureCoords[0][i].x;
			v = importedMesh->mTextureCoords[0][i].y;
		}
		
		const Vertex vertex(
			px, py, pz,
			nx, ny, nz,
			tx, ty, tz,
			u, v
		);

		vertices.push_back(vertex);
	}

	std::vector<uint32_t> indices;
	indices.reserve(importedMesh->mNumFaces * 3ull);

	for (UINT i = 0; i < importedMesh->mNumFaces; ++i)
	{
		const aiFace face = importedMesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	const UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
	const UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(uint32_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mesh->VertexBufferCPU));
	CopyMemory(mesh->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mesh->IndexBufferCPU));
	CopyMemory(mesh->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mesh->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(
		_Device,
		_CommandList,
		vertices.data(),
		vbByteSize,
		mesh->VertexBufferUploader);

	mesh->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(
		_Device,
		_CommandList,
		indices.data(),
		ibByteSize,
		mesh->IndexBufferUploader);

	mesh->VertexByteStride = sizeof(Vertex);
	mesh->VertexBufferByteSize = vbByteSize;
	mesh->IndexFormat = DXGI_FORMAT_R32_UINT;
	mesh->IndexBufferByteSize = ibByteSize;

	int meshIndex = AssetManager::Get().AddMesh(mesh);
	if (meshIndex == INVALID_INDEX)
	{
		LOG_ERROR("Failed to load mesh from file [{0}]", path);
		return INVALID_INDEX;
	}

	Model model;
	model.VertexBufferView = mesh->VertexBufferView();
	model.IndexBufferView = mesh->IndexBufferView();
	model.Name = mesh->Name;
	model.IndexCount = (UINT)indices.size();
	model.StartIndexLocation = 0;
	model.BaseVertexLocation = 0;
	
	int modelIndex = AssetManager::Get().AddModel(model);
	if (modelIndex == INVALID_INDEX)
	{
		LOG_ERROR("Failed to create mode from mesh [{0}]", path);
		return INVALID_INDEX;
	}

	return modelIndex;
}

int AssetManager::AddMesh(std::shared_ptr<Mesh>& mesh)
{
	if (mesh->Name.size() == 0)
	{
		LOG_ERROR("Mesh has no name!", mesh->Name);
		return INVALID_INDEX;
	}
	_Meshes.push_back(mesh);

	return (int)_Meshes.size()-1;
}

//std::shared_ptr<Mesh> AssetManager::GetMesh(ModelType type, const std::string& name)
//{
//	return _Meshes[type][GetMeshIndex(type, name)];
//}
//
//size_t AssetManager::GetMeshIndex(ModelType type, const std::string& name)
//{
//	if (!HasMesh(type, name))
//	{
//		LOG_ERROR("Mesh [{0}] NOT FOUND in AssetManager", name);
//		return 0;
//	}
//	return _MeshNameIndexMap[type].find(name)->second;
//}

// MODEL

//bool AssetManager::HasModel(ModelType type, const std::string& name)
//{
//	return _ModelNameIndexMap[type].find(name) != _ModelNameIndexMap[type].end();
//}

int AssetManager::AddModel(Model& model)
{
	if (model.Name.size() == 0)
	{
		LOG_ERROR("Model has no name!");
		return INVALID_INDEX;
	}

	_Models.push_back(model);

	return (int) _Models.size()-1;
}

Model AssetManager::GetModel(int index)
{
	assert(index != INVALID_INDEX && index < _Models.size() && "Model index out of bounds");
	return _Models[index];
}

