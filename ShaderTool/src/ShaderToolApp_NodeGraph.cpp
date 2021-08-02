#include "pch.h"
#include "ShaderToolApp.h"
#include "AssetManager.h"

#include "Editor\UiNode\AddNode.h"
#include "Editor\UiNode\MultiplyNode.h"
#include "Editor\UiNode\DrawNode.h"
#include "Editor\UiNode\PrimitiveNode.h"
#include "Editor\UiNode\RenderTargetNode.h"
#include "Editor\UiNode\ShaderNode.h"
#include "Editor\UiNode\SineNode.h"
#include "Editor\UiNode\TimeNode.h"

//#include "Events/Event.h"
//#include "Events/EventManager.h"

#include <iomanip>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>

#define VK_S 0x53
#define VK_L 0x4C
#define VK_N 0x4E

using Microsoft::WRL::ComPtr;
using namespace DirectX;

void mini_map_node_hovering_callback(int nodeId, void* userData)
{
	ImGui::SetTooltip("node id %d", nodeId);
}

ImGuiWindowFlags overlay_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

static bool showDfsDebug = false;
static bool showShadersDebug = false;
std::stack<int> postOrderClone; // TODO: debug only

void ShowHoverDebugInfo(std::function<void(void)> info)
{
	static int corner = 0;
	const float XPAD = 20.0f;
	const float YPAD = 40.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - XPAD) : (work_pos.x + XPAD);
	window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - YPAD) : (work_pos.y + YPAD);
	window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
	window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowViewport(viewport->ID);
	bool show = true;
	ImGui::Begin("Hover Debug Info", &show, overlay_window_flags);
	info();
	ImGui::End();
}

void DebugInfo(ShaderToolApp* app)
{
	{
		static bool open = true;
		ImGui::Begin("Debug", &open, overlay_window_flags);
		ImGui::Text("UiNodes: %i", app->GetUiNodes().size());
		ImGui::Text("Nodes:   %i", app->GetGraph().GetNodesCount());
		ImGui::Text("Edges:   %i", app->GetGraph().GetEdgesCount());
		ImGui::End();
	}

	{
		int nodeId;
		if (ImNodes::IsNodeHovered(&nodeId))
		{
			UiNode* node = app->GetUiNode(nodeId);
			if (node)
			{
				switch (node->Type)
				{
				case UiNodeType::Add:
				{
					auto addNode = static_cast<AddNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("AddNode");
						ImGui::Text("Id:      %i", addNode->Id);
						ImGui::Text("Left:    %.5f", addNode->Left);
						ImGui::Text("Right:   %.5f", addNode->Right);
						ImGui::Text("Output:  %.5f", addNode->GetPinValue(addNode->Id));
						});
				}
				break;

				case UiNodeType::Time:
				{
					auto timeNode = static_cast<TimeNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("TimeNode");
						ImGui::Text("Id:      %i", timeNode->Id);
						ImGui::Text("Time:    %.5f", timeNode->GetPinValue(timeNode->Id));
					});
				}
				break;

				case UiNodeType::Sine:
				{
					auto sineNode = static_cast<SineNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("SineNode");
						ImGui::Text("Id:      %i", sineNode->Id);
						ImGui::Text("Sine:    %.5f", sineNode->GetPinValue(sineNode->Id));
					});
				}
				break;

				case UiNodeType::Multiply:
				{
					auto multNode = static_cast<MultiplyNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("MultiplyNode");
						ImGui::Text("Id:      %i", multNode->Id);
						ImGui::Text("Left:    %.5f", multNode->Left);
						ImGui::Text("Right:   %.5f", multNode->Right);
						ImGui::Text("Output:  %.5f", multNode->GetPinValue(multNode->Id));
					});
				}
				break;

				case UiNodeType::Draw:
				{
					auto drawNode = static_cast<DrawNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("DrawNode");
						ImGui::Text("Id:      %i", drawNode->Id);
						ImGui::Text("Shader:  %i", (int)drawNode->GetPinValue(drawNode->ShaderPin));
						ImGui::Text("Model:   %i", (int)drawNode->GetPinValue(drawNode->ModelPin));
						ImGui::Text("Output:  %i", (int)drawNode->GetPinValue(drawNode->Id));
						});
				}
				break;

				case UiNodeType::Primitive:
				{
					auto primNode = static_cast<PrimitiveNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("PrimitiveNode");
						ImGui::Text("Id:            %i", primNode->Id);
						ImGui::Text("SelectedModel: %i", (int)primNode->GetPinValue(primNode->Id));
					});
				}
				break;

				case UiNodeType::Shader:
				{
					auto shaderNode = static_cast<ShaderNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("ShaderNode");
						ImGui::Text("Id:     %i", shaderNode->Id);
						ImGui::Text("Index:  %i", shaderNode->GetPinValue(shaderNode->Id));
						ImGui::Text("Name:   %s", shaderNode->Data.shaderName.c_str());
						ImGui::Text("Path:   %s", shaderNode->Data.path.c_str());
					});
				}
				break;

				default:
					break;
				}
			}
		}
	}

	{
		int linkId;
		if (ImNodes::IsLinkHovered(&linkId))
		{
			ShowHoverDebugInfo([&]() {
				ImGui::Text("Link");
				ImGui::Text("Id: %i", linkId);
				});
		}
	}

	{
		int pinId;
		if (ImNodes::IsPinHovered(&pinId))
		{
			ShowHoverDebugInfo([&]() {
				ImGui::Text("Pin");
				ImGui::Text("Id: %i", pinId);
				});
		}
	}

	//if (ImGui::IsKeyReleased(VK_RETURN))
	//{
	//	auto& shaderMgr = ShaderManager::Get();

	//	for (auto& shader : shaderMgr.GetShaders())
	//	{
	//		shader->PrintDebugInfo();
	//	}
	//}

	if (ImGui::IsKeyReleased(VK_F1))
		showDfsDebug = !showDfsDebug;

	if (showDfsDebug)
	{
		while (!postOrderClone.empty())
		{
			const int id = postOrderClone.top();
			postOrderClone.pop();
			const Node node = app->GetGraph().GetNode(id);

			ImGui::Begin("DFS Debug", &showDfsDebug, overlay_window_flags);
			ImGui::Text("Id: %i |", id); ImGui::SameLine();
			ImGui::Text("Val: %.5f |", node.Value); ImGui::SameLine();
			ImGui::Text("Type: %i |", node.Type); ImGui::SameLine();
			ImGui::Text("TName: %s |", node.TypeName.c_str());
			ImGui::End();
		}
	}

	if (ImGui::IsKeyReleased(VK_F2))
		showShadersDebug = !showShadersDebug;

	if(showShadersDebug)
	{
		auto& shaderMgr = ShaderManager::Get();
		int i = 0;
		for (auto& shader : shaderMgr.GetShaders())
		{
			ImGui::Begin("Shaders Debug", &showDfsDebug, overlay_window_flags);
			ImGui::Text("Index: %i |", i); ImGui::SameLine();
			ImGui::Text("Name: %s |", shader->GetName().c_str()); 
			ImGui::End();
			++i;
		}
	}


#if 0
	if (ImGui::IsKeyReleased(VK_RETURN))
	{
		LOG_TRACE("###########################");
		auto& ids = _Graph.GetNodes();
		LOG_TRACE("Node ids:");
		for (auto id : ids)
			LOG_TRACE("  {0}", id);

		auto edges = _Graph.GetEdges();
		LOG_TRACE("Edge ids:");
		for (auto it = edges.begin(); it != edges.end(); ++it)
			LOG_TRACE("  {0}, {1}, {2}", it->id, it->from, it->to);

		{
			auto edgesFromNodeIds = _Graph.GetEdgesFromNodeIds();
			auto edgesFromNode = _Graph.GetEdgesFromNode();
			LOG_TRACE("EdgesFromNode:");
			int i = 0;
			for (auto it = edgesFromNode.begin(); it != edgesFromNode.end(); ++it, ++i)
				LOG_TRACE("  {0} {1}", edgesFromNodeIds[i], *it);
		}

		{
			auto neighborIds = _Graph.GetAllNeighborIds();
			auto neighbors = _Graph.GetAllNeighbors();
			LOG_TRACE("Neighbors:");
			int i = 0;
			for (auto it = neighbors.begin(); it != neighbors.end(); ++it, ++i)
				for (auto neighborId : *it)
					LOG_TRACE("  {0} {1}", neighborIds[i], neighborId);
		}

	}
#endif
}

void ShaderToolApp::EvaluateGraph()
{
	// MUST set the PSO every frame due to the FrameResource swapping
	_CurrFrameResource->RenderTargetPSO = _RenderTargetPSO;

	if (_RootNodeId == INVALID_ID)
		return;

	std::stack<int> postorder;
	dfs_traverse(_Graph, _RootNodeId, [&postorder](const int nodeId) -> void { postorder.push(nodeId); });

	postOrderClone = postorder;
	//LOG_TRACE("##############################");

	std::stack<float> valueStack;
	while (!postorder.empty())
	{
		const int id = postorder.top();
		postorder.pop();
		Node& node = _Graph.GetNode(id);

		switch (node.Type)
		{
		case NodeType::Value:
		case NodeType::Link:
		{
			// If the edge does not have an edge connecting to another node, then just use the value
			// at this node. It means the node's input pin has not been connected to anything and
			// the value comes from the node's UI.
			if (_Graph.GetNumEdgesFromNode(id) == 0ull)
				valueStack.push(node.Value);
		}
		break;

		case NodeType::Add:
		{
			const float rhs = valueStack.top();
			valueStack.pop();
			const float lhs = valueStack.top();
			valueStack.pop();
			node.Value = lhs + rhs;
			valueStack.push(node.Value);
		}
		break;

		case NodeType::Multiply:
		{
			const float rhs = valueStack.top();
			valueStack.pop();
			const float lhs = valueStack.top();
			valueStack.pop();
			node.Value = rhs * lhs;
			valueStack.push(node.Value);
		}
		break;

		case NodeType::Sine:
		{
			const float input = valueStack.top();
			valueStack.pop();
			node.Value = std::abs(std::sin(input));
			valueStack.push(node.Value);
		}
		break;

		case NodeType::Time:
		{
			valueStack.push(node.Value);
		}
		break;

		//case NodeType::RenderTarget:
		//{
		//	assert(valueStack.size() == 1ull && "RenderTarget node expects 1 input");

		//	const int i = static_cast<int>(valueStack.top());
		//	valueStack.pop();

		//	// it should receive an index with the texture rendered by draw node
		//	if (i > 0)
		//		_RenderTargetReady = true;
		//	else
		//		ClearRenderTexture();
		//}
		//break;
		
		case NodeType::Draw:
		{
			//LOG_ERROR("------ NodeType::Draw");
			//auto valueStackClone = valueStack;
			//while (!valueStackClone.empty())
			//{
			//	LOG_ERROR("NodeType::Draw {0}", valueStackClone.top());
			//	valueStackClone.pop();
			//}
			auto drawNode = static_cast<DrawNode*>(_UINodeIdMap[id]);
			size_t numExpectedInputs = drawNode->ShaderBindingPins.size() + (size_t)2u; // bindings + fixed pins

			if (valueStack.size() != numExpectedInputs)
			{
				LOG_ERROR("Draw node expects {0} inputs but got {1}", numExpectedInputs, valueStack.size());
				assert(valueStack.size() == numExpectedInputs && "Invalid number of DrawNode inputs");
			}
			
			for (auto& bind : drawNode->ShaderBindingPins)
			{
				drawNode->SetPinValue(bind.PinId, valueStack.top());
				valueStack.pop();
			}

			drawNode->SetPinValue(drawNode->ModelPin, valueStack.top());
			valueStack.pop();
			
			drawNode->SetPinValue(drawNode->ShaderPin, valueStack.top());
			valueStack.pop();

			RenderToTexture(drawNode);

			drawNode->SetPinValue(drawNode->Id, 1.f); // TODO: fixed 1 as only one texture is allowed at the moment
			valueStack.push(drawNode->GetPinValue(drawNode->Id)); // TODO: index where the texture is stored, now only one texture
		}
		break;

		case NodeType::Primitive:
		{
			valueStack.push(node.Value);
		}
		break;

		case NodeType::Shader:
		{
			valueStack.push(node.Value);
		}
		break;
		
		default:
			LOG_WARN("NodeType {0} not handled", node.Type);
			break;
		}
	}
}

void ShaderToolApp::BuildRenderTargetRootSignature(const std::string& shaderName)
{
	auto shader = ShaderManager::Get().GetShader(shaderName);	
	auto& rootParameters = shader->GetRootParameters();
	auto staticSamplers = D3DUtil::GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		(UINT)rootParameters.size(),
		rootParameters.data(),
		(UINT)staticSamplers.size(),
		staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(),
		errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
		LOG_ERROR("Error at D3D12SerializeRootSignature() {0}", (char*)errorBlob->GetBufferPointer());

	ThrowIfFailed(hr);

	ThrowIfFailed(
		_Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(_RenderTargetRootSignature.GetAddressOf())));
}

void ShaderToolApp::CreateRenderTargetPSO(int shaderIndex)
{
	LOG_INFO("ShaderToolApp::CreateRenderTargetPSO [{0}]", shaderIndex);

	_InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	D3D12_INPUT_LAYOUT_DESC inputLayout = { _InputLayout.data(), (UINT)_InputLayout.size() };

	auto shaderName = shaderIndex == NOT_LINKED ? DEFAULT_SHADER : ShaderManager::Get().GetShaderName((size_t)shaderIndex);

	BuildRenderTargetRootSignature(shaderName);

	_RenderTargetPSO = std::make_shared<PipelineStateObject>(_BackBufferFormat, _DepthStencilFormat, _4xMsaaState, _4xMsaaQuality);
	_RenderTargetPSO->Create(
		_Device.Get(),
		_RenderTargetRootSignature.Get(),
		inputLayout,
		shaderName);

	_CurrFrameResource->RenderTargetPSO = _RenderTargetPSO;
}

static int PreviousShaderIndexRT = -1;

void ShaderToolApp::RenderToTexture(DrawNode* drawNode)
{
	ClearRenderTexture();

	int currentModel = (int)drawNode->GetPinValue(drawNode->ModelPin);
	if (currentModel == INVALID_INDEX) return;

	int currentShaderIndex = (int)drawNode->GetPinValue(drawNode->ShaderPin);
	if (currentShaderIndex == INVALID_INDEX) return;

	// when shader is changed, it has to recreate the PSO
	if(PreviousShaderIndexRT != currentShaderIndex)
	//if (drawNode->IsDirty)
	{
		CreateRenderTargetPSO(currentShaderIndex);
		PreviousShaderIndexRT = currentShaderIndex;
	}
	
	_CommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			_RenderTarget->GetResource(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	_CommandList->SetGraphicsRootSignature(_RenderTargetRootSignature.Get());
	_CommandList->SetPipelineState(_CurrFrameResource->RenderTargetPSO.get()->GetPSO());
	_CommandList->RSSetViewports(1, &_RenderTarget->GetViewPort());
	_CommandList->RSSetScissorRects(1, &_RenderTarget->GetScissorRect());
	_CommandList->ClearRenderTargetView(_RenderTarget->RTV(), _RenderTarget->GetClearColor(), 0, nullptr);
	_CommandList->OMSetRenderTargets(1, &_RenderTarget->RTV(), true, nullptr);

	auto& bindVars = ShaderManager::Get().GetShader(currentShaderIndex)->GetBindingVars();

	for (auto& [rootParIndex, vars] : bindVars)
	{
		for (auto& var : vars)
		{
			float* data = nullptr;
			// TODO: TEMPORARY!!! Only for testing. It should be properly managed
			if (var.VarName == "World")
			{
				data = &drawNode->_World._11;
			}
			else if (var.VarName == "View")
			{
				data = &drawNode->_View._11;
			}
			else if (var.VarName == "Proj")
			{
				data = &drawNode->_Proj._11;
			}
			else if (var.VarName == "Color")
			{
				XMFLOAT3 color(1.f, 1.f, 0.f);
				data = &color.x;
			}
			
			_CommandList->SetGraphicsRoot32BitConstants(rootParIndex, var.VarNum32BitValues, data, var.VarNum32BitValuesOffset);
		}
	}

	// BOX
	{
		auto& selectedModel = AssetManager::Get().GetModel(_Primitives[currentModel]);
		
		_CommandList->IASetVertexBuffers(0, 1, &selectedModel.VertexBufferView);
		_CommandList->IASetIndexBuffer(&selectedModel.IndexBufferView);
		_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		_CommandList->DrawIndexedInstanced(
			selectedModel.IndexCount,
			1,
			selectedModel.StartIndexLocation,
			selectedModel.BaseVertexLocation,
			0);
	}

	_CommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			_RenderTarget->GetResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ));
}

void ShaderToolApp::ClearRenderTexture()
{
	_CommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			_RenderTarget->GetResource(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	_CommandList->SetGraphicsRootSignature(_RenderTargetRootSignature.Get());
	_CommandList->SetPipelineState(_CurrFrameResource->RenderTargetPSO.get()->GetPSO());
	_CommandList->RSSetViewports(1, &_RenderTarget->GetViewPort());
	_CommandList->RSSetScissorRects(1, &_RenderTarget->GetScissorRect());
	_CommandList->ClearRenderTargetView(_RenderTarget->RTV(), _RenderTarget->GetClearColor(), 0, nullptr);
	_CommandList->OMSetRenderTargets(1, &_RenderTarget->RTV(), true, nullptr);

	_CommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			_RenderTarget->GetResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ));
}

void ShaderToolApp::HandleNewNodes()
{
	// Handle new nodes
	{
		const bool open_popup = (ImGui::IsWindowHovered() || ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) &&
			ImNodes::IsEditorHovered() &&
			ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Right);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
		if (!ImGui::IsAnyItemHovered() && open_popup)
		{
			ImGui::OpenPopup("Add Node");
		}

		// ADD NEW UI NODE

		if (ImGui::BeginPopup("Add Node"))
		{
			const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

			if (ImGui::MenuItem("Add"))
			{
				auto node = std::make_unique<AddNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Multiply"))
			{
				auto node = std::make_unique<MultiplyNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Draw") && _RootNodeId == INVALID_ID)
			{
				auto node = std::make_unique<DrawNode>(&_Graph);
				node->OnCreate();
				_RootNodeId = node->Id;
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Sine"))
			{
				auto node = std::make_unique<SineNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Time"))
			{
				auto node = std::make_unique<TimeNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Render Target") )
			{
				auto node = std::make_unique<RenderTargetNode>(&_Graph, _RenderTarget.get());
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Primitive"))
			{
				auto node = std::make_unique<PrimitiveNode>(&_Graph, _Primitives);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Shader"))
			{
				auto node = std::make_unique<ShaderNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}
}

void ShaderToolApp::HandleNewLinks()
{
	// Handle new links
	// These are driven by Imnodes, so we place the code after EndNodeEditor().
	int start_attr, end_attr;
	if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
	{
		const NodeType start_type = _Graph.GetNode(start_attr).Type;
		const NodeType end_type = _Graph.GetNode(end_attr).Type;

		const bool valid_link = start_type != end_type;
		if (valid_link)
		{
			// Ensure the edge is always directed from the value to
			// whatever produces the value
			if (start_type != NodeType::Value && start_type != NodeType::Link)
			{
				std::swap(start_attr, end_attr);
			}
			_Graph.CreateEdge(start_attr, end_attr);
		}
	}
}

void ShaderToolApp::HandleDeletedLinks()
{
	int link_id;
	if (ImNodes::IsLinkDestroyed(&link_id))
	{
		LOG_WARN("link destroyed {0}", link_id);
		_Graph.EraseEdge(link_id);
	}
	
	const int num_selected = ImNodes::NumSelectedLinks();
	if (num_selected > 0 && ImGui::IsKeyReleased(VK_DELETE))
	{
		static std::vector<int> selected_links;
		selected_links.resize(static_cast<size_t>(num_selected));
		ImNodes::GetSelectedLinks(selected_links.data());
		for (const int edge_id : selected_links)
		{
			_Graph.EraseEdge(edge_id);
		}
	}
}

void ShaderToolApp::HandleDeletedNodes()
{
	const int num_selected = ImNodes::NumSelectedNodes();
	if (num_selected > 0 && ImGui::IsKeyReleased(VK_DELETE))
	{
		static std::vector<int> selected_nodes;
		selected_nodes.resize(static_cast<size_t>(num_selected));
		ImNodes::GetSelectedNodes(selected_nodes.data());
		for (const int node_id : selected_nodes)
		{
			for (auto it = _UINodes.begin(); it != _UINodes.end(); ++it)
			{
				auto node = (*it).get();
				if (node->Id == node_id)
				{
					if (_RootNodeId == node_id) _RootNodeId = INVALID_ID;

					node->OnDelete();
					_UINodeIdMap.erase(node->Id);
					_UINodes.erase(it);
					break;
				}
			}
		}
	}
}

void ShaderToolApp::UpdateNodeGraph()
{
	EVENT_MANAGER.NotifyQueuedEvents();

	for (const auto& node : _UINodes)
		node->OnUpdate(_Timer);
}

void ShaderToolApp::RenderNodeGraph()
{
	// The node editor window
	ImGui::Begin("Shader Editor");
	ImGui::Columns(1);
	ImNodes::BeginNodeEditor();

	HandleNewNodes();

	// RENDER UI NODES
	for (const auto& node : _UINodes)
		node->OnRender();

	for (const auto& edge : _Graph.GetEdges())
	{
		// If edge doesn't start at value, then it's an internal edge, i.e.
		// an edge which links a node's operation to its input. We don't
		// want to render node internals with visible links.
		if (_Graph.GetNode(edge.from).Type != NodeType::Value && _Graph.GetNode(edge.from).Type != NodeType::Link)
			continue;

		ImNodes::Link(edge.id, edge.from, edge.to);
	}

	ImNodes::MiniMap(0.1f, ImNodesMiniMapLocation_BottomRight, mini_map_node_hovering_callback, nullptr);
	ImNodes::EndNodeEditor();

	HandleNewLinks();
	HandleDeletedLinks();
	HandleDeletedNodes();
	
	DebugInfo(this);

	if (ImGui::IsKeyReleased(VK_S))
		Save();

	if (ImGui::IsKeyReleased(VK_L))
		Load();

	if (ImGui::IsKeyReleased(VK_N))
		Reset();

	ImGui::End();
}

void ShaderToolApp::Save()
{
	// Save the internal imnodes state
	ImNodes::SaveCurrentEditorStateToIniFile("node_graph.ini");

	std::ofstream fout("node_graph.txt", std::ios_base::out | std::ios_base::trunc);

	fout << "#graph\n";
	fout << _Graph;
	fout << "#ui_nodes\n";
	fout << _RootNodeId << "\n";
	fout << _UINodes.size() << "\n";

	for (auto& uin : _UINodes)
		fout << "uin " << *uin.get() << "\n";

	fout.close();
}

void ShaderToolApp::Load()
{
	//LOG_TRACE("###################");
	//LOG_TRACE("Loading node graph");

	// Load the internal imnodes state
	ImNodes::LoadCurrentEditorStateFromIniFile("node_graph.ini");

	std::ifstream fin("node_graph.txt", std::ios_base::in);

	if (!fin.is_open())
	{
		LOG_WARN("Failed to load file node_graph.txt");
		return;
	}

	Reset();

	fin >> _Graph;

	std::string comment;
	int numUiNodes;
	fin >> comment >> _RootNodeId >> numUiNodes;

	std::string uinLabel;
	int type;

	for (int i = 0; i < numUiNodes; ++i)
	{
		fin >> uinLabel >> type;
		auto nodeType = static_cast<UiNodeType>(type);
		
		// TODO: it should be implemented using the pattern described in the links below... for now I'll use a switch
		// https://isocpp.org/wiki/faq/serialization#serialize-inherit-no-ptrs
		// https://stackoverflow.com/questions/3268801/how-do-you-de-serialize-a-derived-class-from-serialized-data
		switch (nodeType)
		{
		case UiNodeType::Add: 
		{
			auto node = std::make_unique<AddNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Multiply:
		{
			auto node = std::make_unique<MultiplyNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Draw:
		{
			auto node = std::make_unique<DrawNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Sine:
		{
			auto node = std::make_unique<SineNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Primitive:
		{
			auto node = std::make_unique<PrimitiveNode>(&_Graph, _Primitives);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::RenderTarget:
		{
			auto node = std::make_unique<RenderTargetNode>(&_Graph, _RenderTarget.get());
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Time:
		{
			auto node = std::make_unique<TimeNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Shader:
		{
			auto node = std::make_unique<ShaderNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		
		default:
			LOG_WARN("Failed to load Unknown UI node type {0}", nodeType);
			break;
		}
	}

	fin.close();
}

void ShaderToolApp::Reset()
{
	_Graph.Reset();
	_UINodeIdMap.clear();
	_UINodes.clear();
	_RootNodeId = INVALID_ID;
}
