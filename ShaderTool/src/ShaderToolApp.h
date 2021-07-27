#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "GameTimer.h"
#include "Entity.h"

#include "Rendering\D3DApp.h"
#include "Rendering\RenderTexture.h"
#include "Rendering\Vertex.h"
#include "Rendering\Mesh.h"
#include "Rendering\ShaderManager.h"
#include "Rendering\PipelineStateObject.h"

#include "Editor\Graph\Node.h"
#include "Editor\UiNode\UiNode.h"
#include "Editor\Graph\Graph.h"

struct Primitive
{
	uint16_t Id;
	std::string Name;
};

class ShaderToolApp : public D3DApp
{
public:
	ShaderToolApp(HINSTANCE hInstance);
	ShaderToolApp(const ShaderToolApp& other) = delete;
	void operator=(const ShaderToolApp& other) = delete;
	~ShaderToolApp();

	bool Init();
	void Run();
	void RenderUIDockSpace();
	void UpdateCamera();
	void UpdatePerFrameCB();
	void UpdatePerObjectCB();
	void UpdateNodeGraph();
	void RenderNodeGraph();

	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnResize(uint32_t width, uint32_t height) override;
	virtual void OnKeyDown(WPARAM key) override;
	virtual void OnKeyUp(WPARAM key) override;
	
	void SwapFrameResource();
	void CreateDescriptorHeaps();
	void BuildRootSignature();
	void BuildBackBufferPSO();
	void LoadPrimitiveModels();
	
	const GameTimer& GetTimer() const { return _Timer; }
	const std::vector<std::unique_ptr<UiNode>>& GetUiNodes() const { return _UINodes; }
	const Graph<Node>& GetGraph() const { return _Graph; }
	UiNode* GetUiNode(NodeId id) { return _UINodeIdMap[id]; }

private:
	// TODO: Camera stuff
	DirectX::XMFLOAT3 _EyePos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4 _View = D3DUtil::Identity4x4();
	DirectX::XMFLOAT4X4 _Proj = D3DUtil::Identity4x4();
	DirectX::XMFLOAT4X4 _RTProj = D3DUtil::Identity4x4();
	float _Theta = 1.5f * DirectX::XM_PI;
	float _Phi = DirectX::XM_PIDIV2 - 0.1f;
	float _Radius = 50.0f;
	//

	FrameConstants _FrameCB;
	GameTimer _Timer;
	bool _IsRunning{ true };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _RootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _BackBufferPSO;
	std::vector<D3D12_INPUT_ELEMENT_DESC> _InputLayout;
	std::shared_ptr<PipelineStateObject> _RenderTargetPSO;
	
private: // Node Graph	
	bool _RenderTargetReady{ false };
	std::unique_ptr<RenderTexture> _RenderTarget;  // TODO: at the moment only 1 render target supported
	std::vector<const char*> _Primitives;
	std::vector<std::unique_ptr<UiNode>> _UINodes;
	std::unordered_map<NodeId, UiNode*> _UINodeIdMap;
	Graph<Node> _Graph;
	int _RootNodeId;
	Entity _Entity;
	
	void InitNodeGraph();
	void Reset();
	void Save();
	void Load();
	void EvaluateGraph();
	void RenderToTexture();
	void ClearRenderTexture();
	void HandleNewNodes();
	void HandleNewLinks();
	void HandleDeletedLinks();
	void HandleDeletedNodes();
	void CreateRenderTargetPSO(int shaderIndex);

};