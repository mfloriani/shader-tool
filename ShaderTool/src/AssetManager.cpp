#include "pch.h"
#include "AssetManager.h"
#include "Defines.h"

#include "Rendering/D3DUtil.h"
#include "Rendering/Vertex.h"
#include "Rendering/DDSTextureLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace DirectX;
using namespace D3DUtil;

AssetManager::~AssetManager()
{

}

bool AssetManager::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	_Device = device;
	_CommandList = cmdList;

	return true;
}

void AssetManager::SetTextureDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu, CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu)
{
	_TexSrvCpuDescHandle = srvCpu;
	_TexSrvGpuDescHandle = srvGpu;
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

std::shared_ptr<Texture> AssetManager::CreateTextureFromFile(const std::string& path)
{
	auto tex = std::make_shared<Texture>();
	tex->Name = ExtractFilename(path);
	tex->Path = path;
	tex->SrvCpuDescHandle = _TexSrvCpuDescHandle; // TODO: the descHandle has to be handled properly, now it's fixed
	tex->SrvGpuDescHandle = _TexSrvGpuDescHandle; // TODO: the descHandle has to be handled properly, now it's fixed
	
	CreateTextureSRV(tex);

	tex->Index = StoreTexture(tex);
	return tex;
}

std::shared_ptr<Texture> AssetManager::CreateTextureFromIndex(size_t index)
{
	auto tex = GetTexture(index);
	CreateTextureSRV(tex);
	return tex;
}

void AssetManager::CreateTextureSRV(std::shared_ptr<Texture> tex)
{
	ThrowIfFailed(
		CreateDDSTextureFromFile12(
			_Device,
			_CommandList,
			AnsiToWString(tex->Path).c_str(),
			tex->Resource,
			tex->UploadHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tex->Resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = tex->Resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	_Device->CreateShaderResourceView(tex->Resource.Get(), &srvDesc, tex->SrvCpuDescHandle);
}

size_t AssetManager::StoreTexture(std::shared_ptr<Texture> tex)
{
	size_t index;
	auto it = _TextureNameIndexMap.find(tex->Name);
	if (it == _TextureNameIndexMap.end()) // new texture
	{
		_Textures.push_back(tex);
		index = _Textures.size() - 1;
		_TextureNameIndexMap[tex->Name] = index;
		_TextureIndexPathMap[index] = tex->Path;
		//LOG_TRACE("Adding texture {0} | {1} | {2}", index, name, path);
	}
	else // existing texture
	{
		index = it->second;
		_Textures[index] = tex;
		_TextureIndexPathMap[index] = tex->Path;
		//LOG_TRACE("Updating texture {0} | {1} | {2}", index, name, path);
	}
	return index;
}

std::shared_ptr<Texture> AssetManager::GetTexture(size_t index)
{
	assert(index < _Textures.size() && "Texture index out of bounds");
	auto texture = _Textures[index];
	//LOG_TRACE("GetTexture() {0} | {1} | {2}", index, texture->Name, texture->Path);
	return texture;
}

const std::string AssetManager::GetTexturePath(size_t index)
{
	return GetTexture(index)->Path;
}

std::ostream& AssetManager::Serialize(std::ostream& out)
{
	auto& textures = Get().GetTextures();
	out << textures.size() << "\n";
	
	for (size_t i = 0; i < textures.size(); ++i)
		out << "tex " << i << " " << textures[i]->Path << "\n";

	return out;
}

std::istream& AssetManager::Deserialize(std::istream& in)
{
	size_t numTextures;
	in >> numTextures;

	size_t index;
	std::string label, path;

	for (size_t i = 0; i < numTextures; ++i)
	{
		in >> label >> index >> path;

		std::string name = ExtractFilename(path);

		auto tex = std::make_shared<Texture>();
		tex->Name = name;
		tex->Path = path;
		tex->SrvCpuDescHandle = _TexSrvCpuDescHandle; 
		tex->SrvGpuDescHandle = _TexSrvGpuDescHandle; 

		StoreTexture(tex);
	}
	return in;
}