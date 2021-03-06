#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "GameTimer.h"


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

struct DrawNode;

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
	void UpdateNodeGraph();
	void RenderNodeGraph();

	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnResize(uint32_t width, uint32_t height) override;
	virtual void OnKeyDown(WPARAM key) override;
	virtual void OnKeyUp(WPARAM key) override;
	
	void SwapFrameResource();
	void CreateDescriptorHeaps();
	void BuildBackBufferPSO();
	void LoadPrimitiveModels();
	
	const GameTimer& GetTimer() const { return _Timer; }
	const std::vector<std::unique_ptr<UiNode>>& GetUiNodes() const { return _UINodes; }
	const Graph& GetGraph() const { return _Graph; }
	UiNode* GetUiNode(NodeId id) { return _UINodeIdMap[id]; }

private:
	GameTimer _Timer;
	bool _IsRunning{ true };

	Microsoft::WRL::ComPtr<ID3D12PipelineState> _BackBufferPSO;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> _BackBufferRootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12RootSignature> _RenderTargetRootSignature{ nullptr };
	std::vector<D3D12_INPUT_ELEMENT_DESC> _InputLayout;
	std::shared_ptr<PipelineStateObject> _RenderTargetPSO;
	
private: // Node Graph	
	Graph _Graph;
	int _RootNodeId;
	bool _RenderTargetReady{ false };
	std::vector<int> _Primitives; // stores indices to the primitive models 
	std::unique_ptr<RenderTexture> _RenderTarget;  // TODO: at the moment only 1 render target supported
	std::vector<std::unique_ptr<UiNode>> _UINodes;
	std::unordered_map<NodeId, UiNode*> _UINodeIdMap;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _TextureSrvDescriptorHeap;

	void InitNodeGraph();
	void Reset();
	void Save();
	void Load();
	void EvaluateGraph();
	void RenderToTexture(DrawNode* drawNode);
	void ClearRenderTexture();
	void HandleNewNodes();
	void HandleNewLinks();
	void HandleDeletedLinks();
	void HandleDeletedNodes();
	void BuildRenderTargetRootSignature(const std::string& shaderName);
	void CreateRenderTargetPSO(int shaderIndex);
	void LoadSrvTexture(int textureIndex);


};