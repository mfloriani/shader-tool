#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "Rendering\D3DApp.h"
#include "GameTimer.h"

class EditorApp : public D3DApp
{
public:
	EditorApp(HINSTANCE hInstance);
	EditorApp(const EditorApp& other) = delete;
	void operator=(const EditorApp& other) = delete;
	~EditorApp();

	bool Init();
	void Run();
	void RenderUIDockSpace();

	virtual void OnUpdate() override;
	virtual void OnRender() override;

	virtual void OnKeyDown(WPARAM key) override;
	virtual void OnKeyUp(WPARAM key) override;
	
	void SwapFrameResource();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();
	void LoadDefaultMeshes();

	const GameTimer& GetTimer() const { return _Timer; }

private:
	GameTimer _Timer;
	bool _IsRunning{ true };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _RootSignature{ nullptr };
	std::vector<D3D12_INPUT_ELEMENT_DESC> _InputLayout;
	std::unordered_map<std::string, std::unique_ptr<D3DUtil::MeshGeometry>> _Meshes;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> _Shaders;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> _PSOs;
};