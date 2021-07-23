#include "pch.h"
#include "ShaderToolApp.h"
#include "AssetManager.h"

#include "Editor\UiNode\AddNode.h"
#include "Editor\UiNode\MultiplyNode.h"
#include "Editor\UiNode\DrawNode.h"
#include "Editor\UiNode\PrimitiveNode.h"
#include "Editor\UiNode\RenderTargetNode.h"
#include "Editor\UiNode\VertexShaderNode.h"
#include "Editor\UiNode\PixelShaderNode.h"
#include "Editor\UiNode\SineNode.h"
#include "Editor\UiNode\TimeNode.h"

#include <iomanip>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>

#define VK_S 0x53
#define VK_L 0x4C
#define VK_N 0x4E

static float current_time_seconds = 0.f;

void mini_map_node_hovering_callback(int nodeId, void* userData)
{
	ImGui::SetTooltip("node id %d", nodeId);
}

void ShaderToolApp::EvaluateGraph()
{
	if (_RootNodeId == INVALID_ID)
		return;

	std::stack<int> postorder;
	dfs_traverse(_Graph, _RootNodeId, [&postorder](const int nodeId) -> void { postorder.push(nodeId); });

	LOG_TRACE("#######");

	std::stack<float> valueStack;
	while (!postorder.empty())
	{
		const int id = postorder.top();
		postorder.pop();
		const Node node = _Graph.GetNode(id);

		LOG_TRACE("{0}", node.TypeName);

		switch (node.Type)
		{
		case NodeType::Value:
		{
			// If the edge does not have an edge connecting to another node, then just use the value
			// at this node. It means the node's input pin has not been connected to anything and
			// the value comes from the node's UI.
			if (_Graph.GetNumEdgesFromNode(id) == 0ull)
			{
				valueStack.push(node.Value);
			}
		}
		break;

		case NodeType::Add:
		{
			const float rhs = valueStack.top();
			valueStack.pop();
			const float lhs = valueStack.top();
			valueStack.pop();
			valueStack.push(lhs + rhs);
		}
		break;

		case NodeType::Multiply:
		{
			const float rhs = valueStack.top();
			valueStack.pop();
			const float lhs = valueStack.top();
			valueStack.pop();
			valueStack.push(rhs * lhs);
		}
		break;

		case NodeType::Sine:
		{
			const float x = valueStack.top();
			valueStack.pop();
			const float res = std::abs(std::sin(x));
			valueStack.push(res);
		}
		break;

		case NodeType::Time:
		{
			valueStack.push(current_time_seconds);
		}
		break;

		case NodeType::RenderTarget:
		{
			const int i = static_cast<int>(valueStack.top());
			valueStack.pop();
			
			//LOG_TRACE("RenderTarget {0}", i);

			// it should receive an index with the texture rendered by draw node
			if (i > 0)
				_RenderTargetReady = true;
			else
				ClearRenderTexture();
		}
		break;

		
		case NodeType::Draw:
		{
			// The final output node isn't evaluated in the loop -- instead we just pop
			// the three values which should be in the stack.
			assert(valueStack.size() == 6ull && "Draw node expects 6 inputs");
			
			const float b = std::clamp(valueStack.top(), 0.f, 1.f);
			valueStack.pop();
			const float g = std::clamp(valueStack.top(), 0.f, 1.f);
			valueStack.pop();
			const float r = std::clamp(valueStack.top(), 0.f, 1.f);
			valueStack.pop();

			const int model = static_cast<int>(valueStack.top());
			valueStack.pop();
			
			const int ps = static_cast<int>(valueStack.top());
			valueStack.pop();

			const int vs = static_cast<int>(valueStack.top());
			valueStack.pop();

			LOG_TRACE("VS: {0}", vs);


			_Entity.Color = { r, g, b };
			// TODO: at the moment only works with primitives, but later there will be loaded models
			_Entity.Model = AssetManager::Get().GetModel(_Primitives[model]);
			
			RenderToTexture();

			valueStack.push(1.f); // TODO: index where the texture is stored, now only one texture
		}
		break;

		case NodeType::Primitive:
		{
			//assert(valueStack.size() == 1ull && "Testing the assert");
			valueStack.push(valueStack.top());
			valueStack.pop();
		}
		break;

		case NodeType::VertexShader:
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
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Multiply"))
			{
				auto node = std::make_unique<MultiplyNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Draw"))
			{
				auto node = std::make_unique<DrawNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Sine"))
			{
				auto node = std::make_unique<SineNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Time"))
			{
				auto node = std::make_unique<TimeNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Render Target") && _RootNodeId == INVALID_ID)
			{
				auto node = std::make_unique<RenderTargetNode>(&_Graph, _RenderTarget.get());
				node->OnCreate();
				_RootNodeId = node->Id;
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Primitive"))
			{
				auto node = std::make_unique<PrimitiveNode>(&_Graph, _Primitives);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Vertex Shader"))
			{
				auto node = std::make_unique<VertexShaderNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			if (ImGui::MenuItem("Pixel Shader"))
			{
				auto node = std::make_unique<PixelShaderNode>(&_Graph);
				node->OnCreate();
				ImNodes::SetNodeScreenSpacePos(node->Id, click_pos);
				_UINodes.push_back(std::move(node));
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}
}

void ShaderToolApp::RenderNodeGraph()
{
	_Timer.Tick();

	// Update timer context
	current_time_seconds = _Timer.TotalTime();

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
		if (_Graph.GetNode(edge.from).Type != NodeType::Value)
			continue;

		ImNodes::Link(edge.id, edge.from, edge.to);
	}

	ImNodes::MiniMap(0.1f, ImNodesMiniMapLocation_BottomRight, mini_map_node_hovering_callback, nullptr);
	ImNodes::EndNodeEditor();

	// Handle new links
	// These are driven by Imnodes, so we place the code after EndNodeEditor().
	{
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
				if (start_type != NodeType::Value)
				{
					std::swap(start_attr, end_attr);
				}
				_Graph.CreateEdge(start_attr, end_attr);
			}
		}
	}

	// Handle deleted links
	{
		int link_id;
		if (ImNodes::IsLinkDestroyed(&link_id))
		{
			_Graph.EraseEdge(link_id);
		}
	}

	{
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

	// Handle deleted nodes
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
						// TODO: find a better way of reseting the root node id
						if(node->Type == UiNodeType::RenderTarget)
							_RootNodeId = INVALID_ID;

						node->OnDelete();
						_UINodes.erase(it);
						break;
					}
				}
			}
		}
	}

	if (ImGui::IsKeyReleased(VK_RETURN))
	{
		auto& shaderMgr = ShaderManager::Get();

		for (auto& shader : shaderMgr.GetShaders())
		{
			shader->PrintDebugInfo();
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

	if (ImGui::IsKeyReleased(VK_S))
	{
		Save();
	}

	if (ImGui::IsKeyReleased(VK_L))
	{
		Load();
	}

	if (ImGui::IsKeyReleased(VK_N))
	{
		Reset();
	}

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
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Multiply:
		{
			auto node = std::make_unique<MultiplyNode>(&_Graph);
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Draw:
		{
			auto node = std::make_unique<DrawNode>(&_Graph);
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Sine:
		{
			auto node = std::make_unique<SineNode>(&_Graph);
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Primitive:
		{
			auto node = std::make_unique<PrimitiveNode>(&_Graph, _Primitives);
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::RenderTarget:
		{
			auto node = std::make_unique<RenderTargetNode>(&_Graph, _RenderTarget.get());
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::Time:
		{
			auto node = std::make_unique<TimeNode>(&_Graph);
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::VertexShader:
		{
			auto node = std::make_unique<VertexShaderNode>(&_Graph);
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		case UiNodeType::PixelShader:
		{
			auto node = std::make_unique<PixelShaderNode>(&_Graph);
			fin >> *node.get();
			_UINodes.push_back(std::move(node));
		}
		break;
		default:
			break;
		}
	}

	fin.close();
}

void ShaderToolApp::Reset()
{
	_Graph.Reset();
	_UINodes.clear();
	_RootNodeId = INVALID_ID;
}
