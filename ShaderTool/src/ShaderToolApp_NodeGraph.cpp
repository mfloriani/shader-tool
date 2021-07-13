#include "pch.h"
#include "ShaderToolApp.h"
#include "AssetManager.h"

#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"

#include <iomanip>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>

#define VK_S 0x53
#define VK_L 0x4C
#define VK_N 0x4E

static float current_time_seconds = 0.f;
//static std::vector<bool> gRenderTextures; // TODO: temporary, do it properly

void mini_map_node_hovering_callback(int nodeId, void* userData)
{
	ImGui::SetTooltip("This is node %d", nodeId);
}

void ShaderToolApp::EvaluateGraph()
{
	if (_RootNodeId == -1)
		return;

	std::stack<int> postorder;
	dfs_traverse(_Graph, _RootNodeId, [&postorder](const int nodeId) -> void { postorder.push(nodeId); });

	std::stack<float> valueStack;
	while (!postorder.empty())
	{
		const int id = postorder.top();
		postorder.pop();
		const Node node = _Graph.GetNode(id);

		switch (node.type)
		{
		case NodeType::Value:
		{
			// If the edge does not have an edge connecting to another node, then just use the value
			// at this node. It means the node's input pin has not been connected to anything and
			// the value comes from the node's UI.
			if (_Graph.GetNumEdgesFromNode(id) == 0ull)
			{
				valueStack.push(node.value);
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
			//	gRenderTextures[i] = true;
		}
		break;

		
		case NodeType::Draw:
		{
			// The final output node isn't evaluated in the loop -- instead we just pop
			// the three values which should be in the stack.
			assert(valueStack.size() == 4ull && "Draw node expects 4 inputs");
			
			const float b = std::clamp(valueStack.top(), 0.f, 1.f);
			valueStack.pop();
			const float g = std::clamp(valueStack.top(), 0.f, 1.f);
			valueStack.pop();
			const float r = std::clamp(valueStack.top(), 0.f, 1.f);
			valueStack.pop();

			const int model = static_cast<int>(valueStack.top());
			valueStack.pop();

			_Entity.Color = { r, g, b };
			
			// TODO: temporary... have to sort it out
			if (model == 0)
				_Entity.Model = AssetManager::Get().GetModel("cube");
			else if (model == 1)
				_Entity.Model = AssetManager::Get().GetModel("sphere");
			else if (model == 2)
				_Entity.Model = AssetManager::Get().GetModel("grid");
			
			RenderToTexture();

			valueStack.push(1.f); // TODO: index where the texture is stored
		}
		break;

		case NodeType::Primitive:
		{
			//assert(valueStack.size() == 1ull && "Testing the assert");
			valueStack.push(valueStack.top());
			valueStack.pop();
		}
		break;
		
		default:
			break;
		}
	}
}


void ShaderToolApp::RenderNodeGraph()
{
	_Timer.Tick();

	// Update timer context
	current_time_seconds = _Timer.TotalTime();

	// The node editor window
	ImGui::Begin("color node editor");
	ImGui::Columns(1);
	ImNodes::BeginNodeEditor();

	// Handle new nodes
	{
		const bool open_popup = (ImGui::IsWindowHovered() || ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) &&
			ImNodes::IsEditorHovered() &&
			ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Right);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
		if (!ImGui::IsAnyItemHovered() && open_popup)
		{
			ImGui::OpenPopup("add node");
		}

		// ADD NEW UI NODE

		if (ImGui::BeginPopup("add node"))
		{
			const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

			if (ImGui::MenuItem("Add"))
			{
				const Node value(NodeType::Value, 0.f);
				const Node op(NodeType::Add);

				UiNode ui_node;
				ui_node.type = UiNodeType::Add;
				ui_node.add.lhs = _Graph.CreateNode(value);
				ui_node.add.rhs = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.add.lhs);
				_Graph.CreateEdge(ui_node.id, ui_node.add.rhs);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("Multiply"))
			{
				const Node value(NodeType::Value, 0.f);
				const Node op(NodeType::Multiply);

				UiNode ui_node;
				ui_node.type = UiNodeType::Multiply;
				ui_node.multiply.lhs = _Graph.CreateNode(value);
				ui_node.multiply.rhs = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.multiply.lhs);
				_Graph.CreateEdge(ui_node.id, ui_node.multiply.rhs);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("Draw"))
			{
				const Node value(NodeType::Value, 0.f);
				const Node out(NodeType::Draw);

				UiNode ui_node;
				ui_node.type = UiNodeType::Draw;
				ui_node.draw.model = _Graph.CreateNode(value);
				ui_node.draw.r = _Graph.CreateNode(value);
				ui_node.draw.g = _Graph.CreateNode(value);
				ui_node.draw.b = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(out);

				_Graph.CreateEdge(ui_node.id, ui_node.draw.model);
				_Graph.CreateEdge(ui_node.id, ui_node.draw.r);
				_Graph.CreateEdge(ui_node.id, ui_node.draw.g);
				_Graph.CreateEdge(ui_node.id, ui_node.draw.b);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("Sine"))
			{
				const Node value(NodeType::Value, 0.f);
				const Node op(NodeType::Sine);

				UiNode ui_node;
				ui_node.type = UiNodeType::Sine;
				ui_node.sine.input = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.sine.input);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("Time"))
			{
				UiNode ui_node;
				ui_node.type = UiNodeType::Time;
				ui_node.id = _Graph.CreateNode(Node(NodeType::Time));

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("Render Target") && _RootNodeId == -1)
			{
				const Node value(NodeType::Value, 0.f);
				const Node op(NodeType::RenderTarget);

				UiNode ui_node;
				ui_node.type = UiNodeType::RenderTarget;
				ui_node.renderTarget.input = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.renderTarget.input);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);

				_RootNodeId = ui_node.id;
			}

			if (ImGui::MenuItem("Primitive"))
			{
				const Node value(NodeType::Value, 0.f);
				const Node op(NodeType::Primitive);

				UiNode ui_node;
				ui_node.type = UiNodeType::Primitive;
				ui_node.primitive.input = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.primitive.input);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}

	// RENDER UI NODES

	for (const UiNode& node : _UINodes)
	{
		switch (node.type)
		{
		case UiNodeType::Add:
		{
			const float node_width = 100.f;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("ADD");
			ImNodes::EndNodeTitleBar();
			{
				ImNodes::BeginInputAttribute(node.add.lhs);
				const float label_width = ImGui::CalcTextSize("left").x;
				ImGui::TextUnformatted("left");
				if (_Graph.GetNumEdgesFromNode(node.add.lhs) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat("##hidelabel", &_Graph.GetNode(node.add.lhs).value, 0.01f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			{
				ImNodes::BeginInputAttribute(node.add.rhs);
				const float label_width = ImGui::CalcTextSize("right").x;
				ImGui::TextUnformatted("right");
				if (_Graph.GetNumEdgesFromNode(node.add.rhs) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat("##hidelabel", &_Graph.GetNode(node.add.rhs).value, 0.01f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginOutputAttribute(node.id);
				const float label_width = ImGui::CalcTextSize("result").x;
				ImGui::Indent(node_width - label_width);
				ImGui::TextUnformatted("result");
				ImNodes::EndOutputAttribute();
			}

			ImNodes::EndNode();
		}
		break;
		case UiNodeType::Multiply:
		{
			const float node_width = 100.0f;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("MULTIPLY");
			ImNodes::EndNodeTitleBar();

			{
				ImNodes::BeginInputAttribute(node.multiply.lhs);
				const float label_width = ImGui::CalcTextSize("left").x;
				ImGui::TextUnformatted("left");
				if (_Graph.GetNumEdgesFromNode(node.multiply.lhs) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat(
						"##hidelabel", &_Graph.GetNode(node.multiply.lhs).value, 0.01f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			{
				ImNodes::BeginInputAttribute(node.multiply.rhs);
				const float label_width = ImGui::CalcTextSize("right").x;
				ImGui::TextUnformatted("right");
				if (_Graph.GetNumEdgesFromNode(node.multiply.rhs) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat(
						"##hidelabel", &_Graph.GetNode(node.multiply.rhs).value, 0.01f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginOutputAttribute(node.id);
				const float label_width = ImGui::CalcTextSize("result").x;
				ImGui::Indent(node_width - label_width);
				ImGui::TextUnformatted("result");
				ImNodes::EndOutputAttribute();
			}

			ImNodes::EndNode();
		}
		break;
		case UiNodeType::Draw:
		{
			const float node_width = 100.0f;
			ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
			ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(45, 126, 194, 255));
			ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(81, 148, 204, 255));
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("DRAW");
			ImNodes::EndNodeTitleBar();

			ImGui::Dummy(ImVec2(node_width, 0.f));
			
			{
				ImNodes::BeginInputAttribute(node.draw.model);
				const float label_width = ImGui::CalcTextSize("model").x;
				ImGui::TextUnformatted("model");
				if (_Graph.GetNumEdgesFromNode(node.draw.model) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					//ImGui::DragFloat("##hidelabel", &_Graph.GetNode(node.draw.r).value, 0.01f, 0.f, 1.0f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginInputAttribute(node.draw.r);
				const float label_width = ImGui::CalcTextSize("r").x;
				ImGui::TextUnformatted("r");
				if (_Graph.GetNumEdgesFromNode(node.draw.r) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat("##hidelabel", &_Graph.GetNode(node.draw.r).value, 0.01f, 0.f, 1.0f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginInputAttribute(node.draw.g);
				const float label_width = ImGui::CalcTextSize("g").x;
				ImGui::TextUnformatted("g");
				if (_Graph.GetNumEdgesFromNode(node.draw.g) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat("##hidelabel", &_Graph.GetNode(node.draw.g).value, 0.01f, 0.f, 1.f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginInputAttribute(node.draw.b);
				const float label_width = ImGui::CalcTextSize("b").x;
				ImGui::TextUnformatted("b");
				if (_Graph.GetNumEdgesFromNode(node.draw.b) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat("##hidelabel", &_Graph.GetNode(node.draw.b).value, 0.01f, 0.f, 1.0f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginOutputAttribute(node.id);
				const float label_width = ImGui::CalcTextSize("output").x;
				ImGui::Indent(node_width - label_width);
				ImGui::TextUnformatted("output");
				ImNodes::EndInputAttribute();
			}

			ImNodes::EndNode();
			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
		}
		break;
		case UiNodeType::Sine:
		{
			const float node_width = 100.0f;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("SINE");
			ImNodes::EndNodeTitleBar();

			{
				ImNodes::BeginInputAttribute(node.sine.input);
				const float label_width = ImGui::CalcTextSize("number").x;
				ImGui::TextUnformatted("number");
				if (_Graph.GetNumEdgesFromNode(node.sine.input) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat("##hidelabel", &_Graph.GetNode(node.sine.input).value, 0.01f, 0.f, 1.0f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginOutputAttribute(node.id);
				const float label_width = ImGui::CalcTextSize("output").x;
				ImGui::Indent(node_width - label_width);
				ImGui::TextUnformatted("output");
				ImNodes::EndInputAttribute();
			}

			ImNodes::EndNode();
		}
		break;
		case UiNodeType::Time:
		{
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("TIME");
			ImNodes::EndNodeTitleBar();

			ImNodes::BeginOutputAttribute(node.id);
			ImGui::Text("output");
			ImNodes::EndOutputAttribute();

			ImNodes::EndNode();
		}
		break;

		case UiNodeType::RenderTarget:
		{
			const float node_width = 100.0f;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("RENDER TARGET");
			ImNodes::EndNodeTitleBar();

			{
				ImNodes::BeginInputAttribute(node.renderTarget.input);
				const float label_width = ImGui::CalcTextSize("input").x;
				ImGui::TextUnformatted("input");
				if (_Graph.GetNumEdgesFromNode(node.renderTarget.input) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();

				static int w = 256;
				static int h = 256;
				//if(_RenderTargetReady)
					ImGui::Image((ImTextureID)_RenderTarget->SRV().ptr, ImVec2((float)w, (float)h));
			}

			ImNodes::EndNode();
		}
		break;

		case UiNodeType::Primitive:
		{
			const float node_width = 100.0f;
			ImNodes::BeginNode(node.id);
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("PRIMITIVE");
			ImNodes::EndNodeTitleBar();

			ImGui::PushItemWidth(node_width);
			const char* items[] = { "Cube", "Sphere", "Plane" };
			auto &inputNode = _Graph.GetNode(node.primitive.input);
			int value = static_cast<int>(inputNode.value); // TODO: this seems weird
			ImGui::Combo("##hidelabel", &value, items, IM_ARRAYSIZE(items));
			ImGui::PopItemWidth();
			inputNode.value = static_cast<float>(value);   // TODO: this seems weird
			
			ImNodes::BeginOutputAttribute(node.id);
			const float label_width = ImGui::CalcTextSize("output").x;
			ImGui::Indent(node_width - label_width);
			ImGui::Text("output");
			ImNodes::EndOutputAttribute();

			ImNodes::EndNode();
		}
		break; 

		}
	}

	for (const auto& edge : _Graph.GetEdges())
	{
		// If edge doesn't start at value, then it's an internal edge, i.e.
		// an edge which links a node's operation to its input. We don't
		// want to render node internals with visible links.
		if (_Graph.GetNode(edge.from).type != NodeType::Value)
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
			const NodeType start_type = _Graph.GetNode(start_attr).type;
			const NodeType end_type = _Graph.GetNode(end_attr).type;

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

	{
		const int num_selected = ImNodes::NumSelectedNodes();
		if (num_selected > 0 && ImGui::IsKeyReleased(VK_DELETE))
		{
			static std::vector<int> selected_nodes;
			selected_nodes.resize(static_cast<size_t>(num_selected));
			ImNodes::GetSelectedNodes(selected_nodes.data());
			for (const int node_id : selected_nodes)
			{
				_Graph.EraseNode(node_id);
				auto iter = std::find_if(
					_UINodes.begin(), _UINodes.end(), [node_id](const UiNode& node) -> bool {
					return node.id == node_id;
				});
				// Erase any additional internal nodes
				switch (iter->type)
				{
				case UiNodeType::Add:
					_Graph.EraseNode(iter->add.lhs);
					_Graph.EraseNode(iter->add.rhs);
					break;
				case UiNodeType::Multiply:
					_Graph.EraseNode(iter->multiply.lhs);
					_Graph.EraseNode(iter->multiply.rhs);
					break;
				case UiNodeType::Draw:
					_Graph.EraseNode(iter->draw.r);
					_Graph.EraseNode(iter->draw.g);
					_Graph.EraseNode(iter->draw.b);
					break;
				case UiNodeType::Sine:
					_Graph.EraseNode(iter->sine.input);
					break;
				case UiNodeType::RenderTarget:
					_Graph.EraseNode(iter->renderTarget.input);
					_RootNodeId = -1;
					break;
				default:
					break;
				}
				_UINodes.erase(iter);
			}
		}
	}


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
		fout << "uin " << uin << "\n";

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

	for (int i = 0; i < numUiNodes; ++i)
	{
		fin >> uinLabel;

		UiNode uiNode;
		fin >> uiNode;
		_UINodes.push_back(uiNode);
	}

	fin.close();
}

void ShaderToolApp::Reset()
{
	_Graph.Reset();
	_UINodes.clear();
	_RootNodeId = -1;
}
