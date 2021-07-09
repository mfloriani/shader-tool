#include "pch.h"
#include "ShaderToolApp.h"

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

void mini_map_node_hovering_callback(int nodeId, void* userData)
{
	ImGui::SetTooltip("This is node %d", nodeId);
}

ImU32 evaluate(const Graph<Node>& graph, const int rootId)
{
	std::stack<int> postorder;
	dfs_traverse(graph, rootId, [&postorder](const int nodeId) -> void { postorder.push(nodeId); });

	std::stack<float> valueStack;
	while (!postorder.empty())
	{
		const int id = postorder.top();
		postorder.pop();
		const Node node = graph.GetNode(id);

		switch (node.type)
		{
		case NodeType::add:
		{
			const float rhs = valueStack.top();
			valueStack.pop();
			const float lhs = valueStack.top();
			valueStack.pop();
			valueStack.push(lhs + rhs);
		}
		break;
		case NodeType::multiply:
		{
			const float rhs = valueStack.top();
			valueStack.pop();
			const float lhs = valueStack.top();
			valueStack.pop();
			valueStack.push(rhs * lhs);
		}
		break;
		case NodeType::sine:
		{
			const float x = valueStack.top();
			valueStack.pop();
			const float res = std::abs(std::sin(x));
			valueStack.push(res);
		}
		break;
		case NodeType::time:
		{
			valueStack.push(current_time_seconds);
		}
		break;
		case NodeType::value:
		{
			// If the edge does not have an edge connecting to another node, then just use the value
			// at this node. It means the node's input pin has not been connected to anything and
			// the value comes from the node's UI.
			if (graph.GetNumEdgesFromNode(id) == 0ull)
			{
				valueStack.push(node.value);
			}
		}
		break;
		default:
			break;
		}
	}

	// The final output node isn't evaluated in the loop -- instead we just pop
	// the three values which should be in the stack.
	assert(valueStack.size() == 3ull);
	const int b = static_cast<int>(255.f * std::clamp(valueStack.top(), 0.f, 1.f) + 0.5f);
	valueStack.pop();
	const int g = static_cast<int>(255.f * std::clamp(valueStack.top(), 0.f, 1.f) + 0.5f);
	valueStack.pop();
	const int r = static_cast<int>(255.f * std::clamp(valueStack.top(), 0.f, 1.f) + 0.5f);
	valueStack.pop();

	return IM_COL32(r, g, b, 255);
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
	// These are driven by the user, so we place this code before rendering the nodes
	{
		const bool open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			ImNodes::IsEditorHovered() &&
			ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Right);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
		if (!ImGui::IsAnyItemHovered() && open_popup)
		{
			ImGui::OpenPopup("add node");
		}

		if (ImGui::BeginPopup("add node"))
		{
			const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

			if (ImGui::MenuItem("add"))
			{
				const Node value(NodeType::value, 0.f);
				const Node op(NodeType::add);

				UiNode ui_node;
				ui_node.type = UiNodeType::add;
				ui_node.add.lhs = _Graph.CreateNode(value);
				ui_node.add.rhs = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.add.lhs);
				_Graph.CreateEdge(ui_node.id, ui_node.add.rhs);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("multiply"))
			{
				const Node value(NodeType::value, 0.f);
				const Node op(NodeType::multiply);

				UiNode ui_node;
				ui_node.type = UiNodeType::multiply;
				ui_node.multiply.lhs = _Graph.CreateNode(value);
				ui_node.multiply.rhs = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.multiply.lhs);
				_Graph.CreateEdge(ui_node.id, ui_node.multiply.rhs);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("output") && _RootNodeId == -1)
			{
				const Node value(NodeType::value, 0.f);
				const Node out(NodeType::output);

				UiNode ui_node;
				ui_node.type = UiNodeType::output;
				ui_node.output.r = _Graph.CreateNode(value);
				ui_node.output.g = _Graph.CreateNode(value);
				ui_node.output.b = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(out);

				_Graph.CreateEdge(ui_node.id, ui_node.output.r);
				_Graph.CreateEdge(ui_node.id, ui_node.output.g);
				_Graph.CreateEdge(ui_node.id, ui_node.output.b);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
				_RootNodeId = ui_node.id;
			}

			if (ImGui::MenuItem("sine"))
			{
				const Node value(NodeType::value, 0.f);
				const Node op(NodeType::sine);

				UiNode ui_node;
				ui_node.type = UiNodeType::sine;
				ui_node.sine.input = _Graph.CreateNode(value);
				ui_node.id = _Graph.CreateNode(op);

				_Graph.CreateEdge(ui_node.id, ui_node.sine.input);

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			if (ImGui::MenuItem("time"))
			{
				UiNode ui_node;
				ui_node.type = UiNodeType::time;
				ui_node.id = _Graph.CreateNode(Node(NodeType::time));

				_UINodes.push_back(ui_node);
				ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}

	for (const UiNode& node : _UINodes)
	{
		switch (node.type)
		{
		case UiNodeType::add:
		{
			const float node_width = 100.f;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("add");
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
		case UiNodeType::multiply:
		{
			const float node_width = 100.0f;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("multiply");
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
		case UiNodeType::output:
		{
			const float node_width = 100.0f;
			ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
			ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(45, 126, 194, 255));
			ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(81, 148, 204, 255));
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("output");
			ImNodes::EndNodeTitleBar();

			ImGui::Dummy(ImVec2(node_width, 0.f));
			{
				ImNodes::BeginInputAttribute(node.output.r);
				const float label_width = ImGui::CalcTextSize("r").x;
				ImGui::TextUnformatted("r");
				if (_Graph.GetNumEdgesFromNode(node.output.r) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat(
						"##hidelabel", &_Graph.GetNode(node.output.r).value, 0.01f, 0.f, 1.0f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginInputAttribute(node.output.g);
				const float label_width = ImGui::CalcTextSize("g").x;
				ImGui::TextUnformatted("g");
				if (_Graph.GetNumEdgesFromNode(node.output.g) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat(
						"##hidelabel", &_Graph.GetNode(node.output.g).value, 0.01f, 0.f, 1.f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}

			ImGui::Spacing();

			{
				ImNodes::BeginInputAttribute(node.output.b);
				const float label_width = ImGui::CalcTextSize("b").x;
				ImGui::TextUnformatted("b");
				if (_Graph.GetNumEdgesFromNode(node.output.b) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat(
						"##hidelabel", &_Graph.GetNode(node.output.b).value, 0.01f, 0.f, 1.0f);
					ImGui::PopItemWidth();
				}
				ImNodes::EndInputAttribute();
			}
			ImNodes::EndNode();
			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
		}
		break;
		case UiNodeType::sine:
		{
			const float node_width = 100.0f;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("sine");
			ImNodes::EndNodeTitleBar();

			{
				ImNodes::BeginInputAttribute(node.sine.input);
				const float label_width = ImGui::CalcTextSize("number").x;
				ImGui::TextUnformatted("number");
				if (_Graph.GetNumEdgesFromNode(node.sine.input) == 0ull)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(node_width - label_width);
					ImGui::DragFloat(
						"##hidelabel", &_Graph.GetNode(node.sine.input).value, 0.01f, 0.f, 1.0f);
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
		case UiNodeType::time:
		{
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("time");
			ImNodes::EndNodeTitleBar();

			ImNodes::BeginOutputAttribute(node.id);
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
		if (_Graph.GetNode(edge.from).type != NodeType::value)
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
				if (start_type != NodeType::value)
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
				case UiNodeType::add:
					_Graph.EraseNode(iter->add.lhs);
					_Graph.EraseNode(iter->add.rhs);
					break;
				case UiNodeType::multiply:
					_Graph.EraseNode(iter->multiply.lhs);
					_Graph.EraseNode(iter->multiply.rhs);
					break;
				case UiNodeType::output:
					_Graph.EraseNode(iter->output.r);
					_Graph.EraseNode(iter->output.g);
					_Graph.EraseNode(iter->output.b);
					_RootNodeId = -1;
					break;
				case UiNodeType::sine:
					_Graph.EraseNode(iter->sine.input);
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

	// The color output window
	const ImU32 color = _RootNodeId != -1 ? evaluate(_Graph, _RootNodeId) : IM_COL32(255, 20, 147, 255);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
	ImGui::Begin("output color");
	ImGui::End();
	ImGui::PopStyleColor();



	static int w = 256;
	static int h = 256;
	ImGui::Begin("Render Target");
	//ImGui::Text("CPU handle = %p", _RenderTexture->SRV().ptr);
	//ImGui::Text("GPU handle = %p", _RenderTexture->SRV().ptr);
	ImGui::Text("size = %d x %d", w, h);
	ImGui::Image((ImTextureID)_RenderTexture->SRV().ptr, ImVec2((float)w, (float)h));
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
