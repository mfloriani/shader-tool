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
#include "Editor\UiNode\ColorNode.h"
#include "Editor\UiNode\CameraNode.h"
#include "Editor\UiNode\ScalarNode.h"
#include "Editor\UiNode\Vector4Node.h"
#include "Editor\UiNode\Vector3Node.h"
#include "Editor\UiNode\Vector2Node.h"
#include "Editor\UiNode\Matrix4x4Node.h"
#include "Editor\UiNode\ModelNode.h"
#include "Editor\UiNode\TextureNode.h"
#include "Editor\UiNode\TransformNode.h"

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
						ImGui::Text("Left:    %i %.3f", addNode->LeftPin, addNode->LeftNodeValue->Data);
						ImGui::Text("Right:   %i %.3f", addNode->RightPin, addNode->RightNodeValue->Data);
						ImGui::Text("Output:  %i %.3f", addNode->OutputPin, addNode->OutputNodeValue->Data);
						});
				}
				break;

				case UiNodeType::Multiply:
				{
					auto multNode = static_cast<MultiplyNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("MultiplyNode");
						ImGui::Text("Id:      %i", multNode->Id);
						ImGui::Text("Left:    %i %.3f", multNode->LeftPin, multNode->LeftNodeValue->Data);
						ImGui::Text("Right:   %i %.3f", multNode->RightPin, multNode->RightNodeValue->Data);
						ImGui::Text("Output:  %i %.3f", multNode->OutputPin, multNode->OutputNodeValue->Data);
						});
				}
				break;

				case UiNodeType::Time:
				{
					auto timeNode = static_cast<TimeNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("TimeNode");
						ImGui::Text("Id:      %i", timeNode->Id);
						ImGui::Text("Output:  %i %.5f", timeNode->OutputPin, timeNode->OutputNodeValue->Data);
					});
				}
				break;

				case UiNodeType::Sine:
				{
					auto sineNode = static_cast<SineNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("SineNode");
						ImGui::Text("Id:      %i", sineNode->Id);
						ImGui::Text("Input:   %i %.5f", sineNode->InputPin, sineNode->InputNodeValue->Data);
						ImGui::Text("Output:  %i %.5f", sineNode->OutputPin, sineNode->OutputNodeValue->Data);
					});
				}
				break;

				

				case UiNodeType::Draw:
				{
					auto drawNode = static_cast<DrawNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("DrawNode");
						ImGui::Text("Id:      %i", drawNode->Id);
						ImGui::Text("Shader:  %i %i", drawNode->ShaderPin, drawNode->ShaderNodeValue->Value);
						ImGui::Text("Model:   %i %i", drawNode->ModelPin, drawNode->ModelNodeValue->Data);

						for (auto& bindPin : drawNode->ShaderBindingPins)
						{
							if(bindPin.Bind.VarTypeName == "float4x4")
								ImGui::Text("%s:   %i", bindPin.Bind.VarName.c_str(), bindPin.PinId);
							else if (bindPin.Bind.VarTypeName == "float4")
							{
								auto float4Data = drawNode->GetNodeValuePtr<XMFLOAT4>(bindPin.PinId)->Data;
								float x = float4Data.x, y = float4Data.y, z = float4Data.z, w = float4Data.w;
								ImGui::Text("%s:   %i %.3f %.3f %.3f %.3f", bindPin.Bind.VarName.c_str(), bindPin.PinId, x, y, z, w);
							}
							else if (bindPin.Bind.VarTypeName == "float3")
							{
								auto float3Data = drawNode->GetNodeValuePtr<XMFLOAT3>(bindPin.PinId)->Data;
								float x = float3Data.x, y = float3Data.y, z = float3Data.z;
								ImGui::Text("%s:   %i %.3f %.3f %.3f", bindPin.Bind.VarName.c_str(), bindPin.PinId, x, y, z);
							}
							else if (bindPin.Bind.VarTypeName == "float2")
							{
								auto float2Data = drawNode->GetNodeValuePtr<XMFLOAT2>(bindPin.PinId)->Data;
								float x = float2Data.x, y = float2Data.y;
								ImGui::Text("%s:   %i %.3f %.3f", bindPin.Bind.VarName.c_str(), bindPin.PinId, x, y);
							}
							else if (bindPin.Bind.VarTypeName == "float")
								ImGui::Text("%s:   %i %.3f", bindPin.Bind.VarName.c_str(), bindPin.PinId, drawNode->GetNodeValuePtr<float>(bindPin.PinId)->Data);
							else if (bindPin.Bind.VarTypeName == "int")
								ImGui::Text("%s:   %i %i", bindPin.Bind.VarName.c_str(), bindPin.PinId, drawNode->GetNodeValuePtr<int>(bindPin.PinId)->Data);
						}

						ImGui::Text("Output:  %i %i", drawNode->OutputPin, drawNode->OutputNodeValue->Data);

						});
				}
				break;

				case UiNodeType::Primitive:
				{
					auto primNode = static_cast<PrimitiveNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("PrimitiveNode");
						ImGui::Text("Id:    %i", primNode->Id);
						ImGui::Text("Model: %i %i", primNode->OutputPin, primNode->OutputNodeValue->Data);
					});
				}
				break;

				case UiNodeType::Shader:
				{
					auto shaderNode = static_cast<ShaderNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("ShaderNode");
						ImGui::Text("Id:     %i", shaderNode->Id);
						ImGui::Text("Name:   %s", shaderNode->GetName().c_str());
						ImGui::Text("Path:   %s", shaderNode->GetPath().c_str());
						ImGui::Text("Output: %i %i", shaderNode->OutputPin, shaderNode->OutputNodeValue->Value);
					});
				}
				break;

				case UiNodeType::Model:
				{
					auto modelNode = static_cast<ModelNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("ShaderNode");
						ImGui::Text("Id:     %i", modelNode->Id);
						ImGui::Text("Name:   %s", modelNode->GetName().c_str());
						ImGui::Text("Path:   %s", modelNode->GetPath().c_str());
						ImGui::Text("Output: %i %i", modelNode->OutputPin, modelNode->OutputNodeValue->Data);
						});
				}
				break;

				case UiNodeType::Color:
				{
					auto colorNode = static_cast<ColorNode*>(node);
					auto float3Value = colorNode->GetNodeValuePtr<XMFLOAT3>(colorNode->OutputPin)->Data;
					float x = float3Value.x, y = float3Value.y, z = float3Value.z;
					ShowHoverDebugInfo([&]() {
						ImGui::Text("ColorNode");
						ImGui::Text("Id:     %i", colorNode->Id);
						ImGui::Text("Color:  %i %.3f %.3f %.3f", colorNode->OutputPin, x, y, z);
						});
				}
				break;

				case UiNodeType::Camera:
				{
					auto uinode = static_cast<CameraNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("ColorNode");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("View:       %i", uinode->ViewPin);
						ImGui::Text("Projection: %i", uinode->ProjectionPin);
						});
				}
				break;

				case UiNodeType::Scalar:
				{
					auto uinode = static_cast<ScalarNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("ScalarNode");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("Output:     %i", uinode->OutputPin);
						});
				}
				break;

				case UiNodeType::Vector4:
				{
					auto uinode = static_cast<Vector4Node*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("Vector4Node");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("X:          %i", uinode->XInputPin);
						ImGui::Text("Y:          %i", uinode->YInputPin);
						ImGui::Text("Z:          %i", uinode->ZInputPin);
						ImGui::Text("W:          %i", uinode->WInputPin);
						ImGui::Text("Output:     %i", uinode->OutputPin);
						});
				}
				break;

				case UiNodeType::Vector3:
				{
					auto uinode = static_cast<Vector3Node*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("Vector3Node");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("X:          %i", uinode->XInputPin);
						ImGui::Text("Y:          %i", uinode->YInputPin);
						ImGui::Text("Z:          %i", uinode->ZInputPin);
						ImGui::Text("Output:     %i", uinode->OutputPin);
						});
				}
				break;

				case UiNodeType::Vector2:
				{
					auto uinode = static_cast<Vector2Node*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("Vector2Node");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("X:          %i", uinode->XInputPin);
						ImGui::Text("Y:          %i", uinode->YInputPin);
						ImGui::Text("Output:     %i", uinode->OutputPin);
						});
				}
				break;

				case UiNodeType::Matrix4x4:
				{
					auto uinode = static_cast<Matrix4x4Node*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("Vector2Node");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("Input:      %i", uinode->InputPin);
						ImGui::Text("Output:     %i", uinode->OutputPin);
						});
				}
				break;

				case UiNodeType::Texture:
				{
					auto uinode = static_cast<TextureNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("Vector2Node");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("Name:       %s", uinode->GetName().c_str());
						ImGui::Text("Path:       %s", uinode->GetPath().c_str());
						ImGui::Text("Output:     %i", uinode->OutputPin);
						});
				}
				break;

				case UiNodeType::Transform:
				{
					auto uinode = static_cast<TransformNode*>(node);
					ShowHoverDebugInfo([&]() {
						ImGui::Text("Vector2Node");
						ImGui::Text("Id:         %i", uinode->Id);
						ImGui::Text("Position:   %i %.3f %.3f %.3f", uinode->PositionPin, uinode->PositionNodeValue->Data.x, uinode->PositionNodeValue->Data.y, uinode->PositionNodeValue->Data.z);
						ImGui::Text("Rotation:   %i %.3f %.3f %.3f", uinode->RotationPin, uinode->RotationNodeValue->Data.x, uinode->RotationNodeValue->Data.y, uinode->RotationNodeValue->Data.z);
						ImGui::Text("Scale:      %i %.3f %.3f %.3f", uinode->ScalePin, uinode->ScaleNodeValue->Data.x, uinode->ScaleNodeValue->Data.y, uinode->ScaleNodeValue->Data.z);
						ImGui::Text("Output:     %i", uinode->OutputPin);
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
			//ImGui::Text("Val: %.5f |", node.Value); ImGui::SameLine();
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

	//std::stack<float> valueStack;
	while (!postorder.empty())
	{
		const int id = postorder.top();
		postorder.pop();
		Node& node = _Graph.GetNode(id);

		switch (node.Type)
		{
		case NodeType::Float:
		case NodeType::Float2:
		case NodeType::Float3:
		case NodeType::Float4:
		case NodeType::Float4x4:
		case NodeType::Int:
		{
			// nothing happen at the moment
		}
		break;

		case NodeType::Add:
		{
			static_cast<AddNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Multiply:
		{
			static_cast<MultiplyNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Sine:
		{
			static_cast<SineNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Time:
		{
			static_cast<TimeNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;
		
		case NodeType::RenderTarget:
		{
			//calling it here although RenderTarget is not part of the graph search anymore...
			static_cast<RenderTargetNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;
		
		case NodeType::Draw:
		{
			auto drawNode = static_cast<DrawNode*>(_UINodeIdMap[id]);
			drawNode->OnEval();
			RenderToTexture(drawNode);
		}
		break;

		case NodeType::Primitive:
		{
			static_cast<PrimitiveNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Shader:
		{
			static_cast<ShaderNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Color:
		{
			static_cast<ColorNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;
		
		case NodeType::Camera:
		{
			static_cast<CameraNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Scalar:
		{
			static_cast<ScalarNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Vector4:
		{
			static_cast<Vector4Node*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Vector3:
		{
			static_cast<Vector3Node*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Vector2:
		{
			static_cast<Vector2Node*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Matrix4x4:
		{
			static_cast<Matrix4x4Node*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Model:
		{
			static_cast<ModelNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Texture:
		{
			static_cast<TextureNode*>(_UINodeIdMap[id])->OnEval();
		}
		break;

		case NodeType::Transform:
		{
			static_cast<TransformNode*>(_UINodeIdMap[id])->OnEval();
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

static int PreviousShaderIndexRT = INVALID_INDEX;

void ShaderToolApp::RenderToTexture(DrawNode* drawNode)
{
	ClearRenderTexture();

	if (drawNode->OutputNodeValue->Data == INVALID_INDEX) return; // NOT READY

	int currentModel = drawNode->ModelNodeValue->Data;
	if (currentModel == INVALID_INDEX) return; // no model linked

	int currentShaderIndex = drawNode->ShaderNodeValue->Value;
	if (currentShaderIndex == INVALID_INDEX) return; // no shader linked

	// recreate PSO when shader has changed
	if(PreviousShaderIndexRT != currentShaderIndex)
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

	for (auto& var : drawNode->ShaderBindingPins)
	{
		_CommandList->SetGraphicsRoot32BitConstants(
			var.Bind.RootParameterIndex, 
			var.Bind.VarNum32BitValues, 
			var.Data, 
			var.Bind.VarNum32BitValuesOffset);
	}

	{
		auto& selectedModel = AssetManager::Get().GetModel(currentModel);
		
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

			if (ImGui::MenuItem("Draw") && _RootNodeId == INVALID_ID)
			{
				auto node = std::make_unique<DrawNode>(&_Graph);
				node->OnCreate();
				_RootNodeId = node->Id;
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Render Target"))
			{
				auto node = std::make_unique<RenderTargetNode>(&_Graph, _RenderTarget.get());
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Shader"))
			{
				auto node = std::make_unique<ShaderNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Camera"))
			{
				auto node = std::make_unique<CameraNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Transform"))
			{
				auto node = std::make_unique<TransformNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Primitive"))
			{
				auto node = std::make_unique<PrimitiveNode>(&_Graph, _Primitives);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Model"))
			{
				auto node = std::make_unique<ModelNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Texture"))
			{
				auto node = std::make_unique<TextureNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Color"))
			{
				auto node = std::make_unique<ColorNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Time"))
			{
				auto node = std::make_unique<TimeNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Add"))
			{
				auto node = std::make_unique<AddNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Multiply"))
			{
				auto node = std::make_unique<MultiplyNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Sine"))
			{
				auto node = std::make_unique<SineNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Scalar"))
			{
				auto node = std::make_unique<ScalarNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Vector4"))
			{
				auto node = std::make_unique<Vector4Node>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Vector3"))
			{
				auto node = std::make_unique<Vector3Node>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Vector2"))
			{
				auto node = std::make_unique<Vector2Node>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodeIdMap[node->Id] = node.get();
				_UINodes.push_back(std::move(node));
			}
			else if (ImGui::MenuItem("Matrix4x4"))
			{
				auto node = std::make_unique<Matrix4x4Node>(&_Graph);
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
		const Node start_node = _Graph.GetNode(start_attr);
		const Node end_node = _Graph.GetNode(end_attr);

		const bool valid_direction = start_node.Direction != end_node.Direction 
			&& start_node.Direction != NodeDirection::None 
			&& end_node.Direction != NodeDirection::None;

		if (!valid_direction)
		{
			LOG_WARN("Failed to create link -> Invalid node directions [start: {0} | end: {1}]", 
				magic_enum::enum_name(start_node.Direction), magic_enum::enum_name(end_node.Direction));
			return;
		}

		const bool valid_type = start_node.Type == end_node.Type;

		if (!valid_type)
		{
			LOG_WARN("Failed to create link -> Invalid node types [start: {0} | end: {1}]", 
				magic_enum::enum_name(start_node.Type), magic_enum::enum_name(end_node.Type));
			return;
		}
		
		// Ensure the edge is always directed from the input pin to
		// whatever produces the value (output pin)
		if (start_node.Direction != NodeDirection::In)
		{
			std::swap(start_attr, end_attr);
		}
		_Graph.CreateEdge(start_attr, end_attr, EdgeType::External);
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

	for (auto& node : _UINodes)
		node->OnUpdate();
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
		//if (_Graph.GetNode(edge.from).Type != NodeType::Value && _Graph.GetNode(edge.from).Type != NodeType::Link)
		if(edge.type == EdgeType::Internal)
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
		fout << *uin.get() << "\n";

	fout.close();
}

void ShaderToolApp::Load()
{
	EVENT_MANAGER.Enable(false); // TODO: awful hack to avoid duplication :/	
	
	ImNodes::LoadCurrentEditorStateFromIniFile("node_graph.ini"); // Load the internal imnodes state
	std::ifstream fin("node_graph.txt", std::ios_base::in); // load project data

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

	std::string typeName;
	int type;

	for (int i = 0; i < numUiNodes; ++i)
	{
		fin >> typeName >> type;
		auto nodeType = static_cast<UiNodeType>(type);
		
		// TODO: could be implemented using the pattern described in the links below... for now I'll use a switch
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
		case UiNodeType::Model:
		{
			auto node = std::make_unique<ModelNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Texture:
		{
			auto node = std::make_unique<TextureNode>(&_Graph);
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
		case UiNodeType::Color:
		{
			auto node = std::make_unique<ColorNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Camera:
		{
			auto node = std::make_unique<CameraNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Scalar:
		{
			auto node = std::make_unique<ScalarNode>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Vector4:
		{
			auto node = std::make_unique<Vector4Node>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Vector3:
		{
			auto node = std::make_unique<Vector3Node>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Vector2:
		{
			auto node = std::make_unique<Vector2Node>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Matrix4x4:
		{
			auto node = std::make_unique<Matrix4x4Node>(&_Graph);
			fin >> *node.get();
			_UINodeIdMap[node->Id] = node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Transform:
		{
			auto node = std::make_unique<TransformNode>(&_Graph);
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

	EVENT_MANAGER.Enable(true);
}

void ShaderToolApp::Reset()
{
	_Graph.Reset();
	_UINodeIdMap.clear();
	_UINodes.clear();
	_RootNodeId = INVALID_ID;
}
