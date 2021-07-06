#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "GameTimer.h"
#include "Rendering\D3DApp.h"
#include "Rendering\RenderTexture.h"

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
	void UpdateCamera();
	void UpdatePerFrameCB();
	void UpdatePerObjectCB();

	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnResize(uint32_t width, uint32_t height) override;
	virtual void OnKeyDown(WPARAM key) override;
	virtual void OnKeyUp(WPARAM key) override;
	
	void SwapFrameResource();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();
	void LoadDefaultMeshes();
	void CreateDescriptorHeaps();

	const GameTimer& GetTimer() const { return _Timer; }

private:
	// TODO: Camera stuff
	DirectX::XMFLOAT3 _EyePos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4 _View = D3DUtil::Identity4x4();
	DirectX::XMFLOAT4X4 _Proj = D3DUtil::Identity4x4();
	float _Theta = 1.5f * DirectX::XM_PI;
	float _Phi = DirectX::XM_PIDIV2 - 0.1f;
	float _Radius = 50.0f;
	//

	FrameConstants _FrameCB;
	GameTimer _Timer;
	bool _IsRunning{ true };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _RootSignature{ nullptr };
	std::vector<D3D12_INPUT_ELEMENT_DESC> _InputLayout;
	std::unordered_map<std::string, std::unique_ptr<D3DUtil::MeshGeometry>> _Meshes;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> _Shaders;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> _PSOs;

	// TODO: move this to the editor?
	// Render-to-texture data
	std::unique_ptr<RenderTexture>               _RenderTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _RTsrvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _RTrtvDescriptorHeap;

};