#pragma once

#include "Graph.h"
#include "GameTimer.h"

#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <vector>
#include <fstream>
#include <iomanip>


#define VK_S 0x53
#define VK_L 0x4C


static GameTimer _Timer;

enum class NodeType
{
    add,
    multiply,
    output,
    sine,
    time,
    value
};

struct Node
{
    NodeType type;
    float    value;

    explicit Node(const NodeType t) : type(t), value(0.f) {}
    Node(const NodeType t, const float v) : type(t), value(v) {}

    friend std::ostream& operator<<(std::ostream& out, const Node& n)
    {
        out << static_cast<int>(n.type) 
            << " " 
            << n.value;
        return out;
    }
};

template<class T>
T clamp(T x, T a, T b)
{
    return std::min(b, std::max(x, a));
}

void mini_map_node_hovering_callback(int node_id, void* user_data)
{
    ImGui::SetTooltip("This is node %d", node_id);
}

static float current_time_seconds = 0.f;
static bool  emulate_three_button_mouse = false;

ImU32 evaluate(const Graph<Node>& graph, const int root_node)
{
    std::stack<int> postorder;
    dfs_traverse(graph, root_node, [&postorder](const int node_id) -> void { postorder.push(node_id); });

    std::stack<float> value_stack;
    while (!postorder.empty())
    {
        const int id = postorder.top();
        postorder.pop();
        const Node node = graph.node(id);

        switch (node.type)
        {
        case NodeType::add:
        {
            const float rhs = value_stack.top();
            value_stack.pop();
            const float lhs = value_stack.top();
            value_stack.pop();
            value_stack.push(lhs + rhs);
        }
        break;
        case NodeType::multiply:
        {
            const float rhs = value_stack.top();
            value_stack.pop();
            const float lhs = value_stack.top();
            value_stack.pop();
            value_stack.push(rhs * lhs);
        }
        break;
        case NodeType::sine:
        {
            const float x = value_stack.top();
            value_stack.pop();
            const float res = std::abs(std::sin(x));
            value_stack.push(res);
        }
        break;
        case NodeType::time:
        {
            value_stack.push(current_time_seconds);
        }
        break;
        case NodeType::value:
        {
            // If the edge does not have an edge connecting to another node, then just use the value
            // at this node. It means the node's input pin has not been connected to anything and
            // the value comes from the node's UI.
            if (graph.num_edges_from_node(id) == 0ull)
            {
                value_stack.push(node.value);
            }
        }
        break;
        default:
            break;
        }
    }

    // The final output node isn't evaluated in the loop -- instead we just pop
    // the three values which should be in the stack.
    assert(value_stack.size() == 3ull);
    const int b = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
    value_stack.pop();
    const int g = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
    value_stack.pop();
    const int r = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
    value_stack.pop();

    return IM_COL32(r, g, b, 255);
}

class ColorNodeEditor
{
public:
    ColorNodeEditor() : graph_(), uiNodes_(), root_node_id_(-1) {}

    void Init()
    {
        _Timer.Reset();

        ImNodesIO& io = ImNodes::GetIO();
        io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

        //Load();
    }

    void Quit()
    {
        //Save();
    }

    void show()
    {
        _Timer.Tick();

        // Update timer context
        current_time_seconds = _Timer.TotalTime();

        // The node editor window
        ImGui::Begin("color node editor");
        //ImGui::TextUnformatted("Edit the color of the output color window using nodes.");
        //ImGui::Columns(2);
        //ImGui::TextUnformatted("A -- add node");
        //ImGui::TextUnformatted("X -- delete selected node or link");
        //ImGui::NextColumn();
        //if (ImGui::Checkbox("emulate_three_button_mouse", &emulate_three_button_mouse))
        //{
        //    ImNodes::GetIO().EmulateThreeButtonMouse.Modifier =
        //        emulate_three_button_mouse ? &ImGui::GetIO().KeyAlt : NULL;
        //}
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
                    ui_node.add.lhs = graph_.insert_node(value);
                    ui_node.add.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.add.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.add.rhs);

                    uiNodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("multiply"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::multiply);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::multiply;
                    ui_node.multiply.lhs = graph_.insert_node(value);
                    ui_node.multiply.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.multiply.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.multiply.rhs);

                    uiNodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("output") && root_node_id_ == -1)
                {
                    const Node value(NodeType::value, 0.f);
                    const Node out(NodeType::output);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::output;
                    ui_node.output.r = graph_.insert_node(value);
                    ui_node.output.g = graph_.insert_node(value);
                    ui_node.output.b = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(out);

                    graph_.insert_edge(ui_node.id, ui_node.output.r);
                    graph_.insert_edge(ui_node.id, ui_node.output.g);
                    graph_.insert_edge(ui_node.id, ui_node.output.b);

                    uiNodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                    root_node_id_ = ui_node.id;
                }

                if (ImGui::MenuItem("sine"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::sine);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::sine;
                    ui_node.sine.input = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.sine.input);

                    uiNodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("time"))
                {
                    UiNode ui_node;
                    ui_node.type = UiNodeType::time;
                    ui_node.id = graph_.insert_node(Node(NodeType::time));

                    uiNodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
        }

        for (const UiNode& node : uiNodes_)
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
                    if (graph_.num_edges_from_node(node.add.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.add.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                {
                    ImNodes::BeginInputAttribute(node.add.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.add.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.add.rhs).value, 0.01f);
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
                    if (graph_.num_edges_from_node(node.multiply.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.multiply.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                {
                    ImNodes::BeginInputAttribute(node.multiply.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.multiply.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.multiply.rhs).value, 0.01f);
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
                    if (graph_.num_edges_from_node(node.output.r) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.output.r).value, 0.01f, 0.f, 1.0f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    ImNodes::BeginInputAttribute(node.output.g);
                    const float label_width = ImGui::CalcTextSize("g").x;
                    ImGui::TextUnformatted("g");
                    if (graph_.num_edges_from_node(node.output.g) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.output.g).value, 0.01f, 0.f, 1.f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    ImNodes::BeginInputAttribute(node.output.b);
                    const float label_width = ImGui::CalcTextSize("b").x;
                    ImGui::TextUnformatted("b");
                    if (graph_.num_edges_from_node(node.output.b) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.output.b).value, 0.01f, 0.f, 1.0f);
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
                    if (graph_.num_edges_from_node(node.sine.input) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.sine.input).value, 0.01f, 0.f, 1.0f);
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

        for (const auto& edge : graph_.edges())
        {
            // If edge doesn't start at value, then it's an internal edge, i.e.
            // an edge which links a node's operation to its input. We don't
            // want to render node internals with visible links.
            if (graph_.node(edge.from).type != NodeType::value)
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
                const NodeType start_type = graph_.node(start_attr).type;
                const NodeType end_type = graph_.node(end_attr).type;

                const bool valid_link = start_type != end_type;
                if (valid_link)
                {
                    // Ensure the edge is always directed from the value to
                    // whatever produces the value
                    if (start_type != NodeType::value)
                    {
                        std::swap(start_attr, end_attr);
                    }
                    graph_.insert_edge(start_attr, end_attr);
                }
            }
        }

        // Handle deleted links
        {
            int link_id;
            if (ImNodes::IsLinkDestroyed(&link_id))
            {
                graph_.erase_edge(link_id);
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
                    graph_.erase_edge(edge_id);
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
                    graph_.erase_node(node_id);
                    auto iter = std::find_if(
                        uiNodes_.begin(), uiNodes_.end(), [node_id](const UiNode& node) -> bool {
                        return node.id == node_id;
                    });
                    // Erase any additional internal nodes
                    switch (iter->type)
                    {
                    case UiNodeType::add:
                        graph_.erase_node(iter->add.lhs);
                        graph_.erase_node(iter->add.rhs);
                        break;
                    case UiNodeType::multiply:
                        graph_.erase_node(iter->multiply.lhs);
                        graph_.erase_node(iter->multiply.rhs);
                        break;
                    case UiNodeType::output:
                        graph_.erase_node(iter->output.r);
                        graph_.erase_node(iter->output.g);
                        graph_.erase_node(iter->output.b);
                        root_node_id_ = -1;
                        break;
                    case UiNodeType::sine:
                        graph_.erase_node(iter->sine.input);
                        break;
                    default:
                        break;
                    }
                    uiNodes_.erase(iter);
                }
            }
        }


        if (ImGui::IsKeyReleased(VK_RETURN))
        {
            LOG_TRACE("###########################");
            auto& ids = graph_.nodes();
            LOG_TRACE("Node ids:");
            for(auto id : ids)
                LOG_TRACE("  {0}", id);

            auto edges = graph_.edges();
            LOG_TRACE("Edge ids:");
            for (auto it = edges.begin(); it != edges.end(); ++it)
                LOG_TRACE("  {0}, {1}, {2}", it->id, it->from, it->to);

            {
                auto edgesFromNodeIds = graph_.edgesFromNodeIds();
                auto edgesFromNode = graph_.edgesFromNode();
                LOG_TRACE("EdgesFromNode:");
                int i = 0;
                for (auto it = edgesFromNode.begin(); it != edgesFromNode.end(); ++it, ++i)
                    LOG_TRACE("  {0} {1}", edgesFromNodeIds[i], *it);
            }

            {
                auto neighborIds = graph_.allNeighborIds();
                auto neighbors = graph_.allNeighbors();
                LOG_TRACE("Neighbors:");
                int i = 0;
                for (auto it = neighbors.begin(); it != neighbors.end(); ++it, ++i)
                    for(auto neighborId : *it)
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

        ImGui::End();

        // The color output window
        const ImU32 color = root_node_id_ != -1 ? evaluate(graph_, root_node_id_) : IM_COL32(255, 20, 147, 255);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
        ImGui::Begin("output color");
        ImGui::End();
        ImGui::PopStyleColor();
    }

    void Save()
    {
        // Save the internal imnodes state
        ImNodes::SaveCurrentEditorStateToIniFile("node_graph.ini");
        
        std::ofstream fout("node_graph.txt", std::ios_base::out | std::ios_base::trunc);

        fout << "#graph\n";
        fout << graph_;
        fout << "#ui_nodes\n";
        fout << root_node_id_ << "\n";
        fout << uiNodes_.size() << "\n";

        for(auto& uin : uiNodes_)
            fout << "uin " << uin << "\n";

        fout.close();
    }

    void Load()
    {
        // Load the internal imnodes state
        ImNodes::LoadCurrentEditorStateFromIniFile("node_graph.ini");

        std::ifstream fin("node_graph.txt", std::ios_base::in);

        if (!fin.is_open())
        {
            LOG_WARN("Failed to load file node_graph.txt");
            return;
        }

        LOG_TRACE("###################");

        std::string comment; // #graph
        int currentId, numNodes;
        fin >> comment >> currentId >> numNodes;

        std::string nLabel;
        int nId, nType;
        float nValue;

        for (int i = 0; i < numNodes; ++i)
        {
            fin >> nLabel >> nId >> nType >> nValue;
            LOG_TRACE("{0} {1} {2} {3}", nLabel, nId, nType, nValue);
        }

        int numEdges;
        fin >> numEdges;

        std::string eLabel;
        int eId, eFrom, eTo;

        for (int i = 0; i < numEdges; ++i)
        {
            fin >> eLabel >> eId >> eFrom >> eTo;
            LOG_TRACE("{0} {1} {2} {3}", eLabel, eId, eFrom, eTo);
        }

        int numEdgesFromNode;
        fin >> numEdgesFromNode;

        std::string enLabel;
        int enId, enCount;

        for (int i = 0; i < numEdgesFromNode; ++i)
        {
            fin >> enLabel >> enId >> enCount;
            LOG_TRACE("{0} {1} {2}", enLabel, enId, enCount);
        }

        int numNeighbors;
        fin >> numNeighbors;

        std::string nbLabel;
        int nbId, nbTo;

        for (int i = 0; i < numNeighbors; ++i)
        {
            fin >> nbLabel >> nbId >> nbTo;
            LOG_TRACE("{0} {1} {2}", nbLabel, nbId, nbTo);
        }

        int rootId, numUiNodes;
        fin >> comment >> rootId >> numUiNodes;

        std::string uinLabel;
        
        for (int i = 0; i < numUiNodes; ++i)
        {
            UiNode uiNode;
            int uinType;
            fin >> uinLabel >> uinType >> uiNode.id;

            uiNode.type = static_cast<UiNodeType>(uinType);

            switch (uiNode.type)
            {
            case UiNodeType::add:
            {
                fin >> uiNode.add.lhs >> uiNode.add.rhs;
                LOG_TRACE("{0} {1} {2} {3} {4}", uinLabel, uiNode.type, uiNode.id, uiNode.add.lhs, uiNode.add.rhs);
                break;
            }
            case UiNodeType::multiply:
            {
                fin >> uiNode.multiply.lhs >> uiNode.multiply.rhs;
                LOG_TRACE("{0} {1} {2} {3} {4}", uinLabel, uiNode.type, uiNode.id, uiNode.multiply.lhs, uiNode.multiply.rhs);
                break;
            }
            case UiNodeType::output:
            {
                fin >> uiNode.output.r >> uiNode.output.g >> uiNode.output.b;
                LOG_TRACE("{0} {1} {2} {3} {4} {5}", uinLabel, uiNode.type, uiNode.id, uiNode.output.r, uiNode.output.g, uiNode.output.b);
                break;
            }
            case UiNodeType::sine:
            {
                fin >> uiNode.sine.input;
                LOG_TRACE("{0} {1} {2} {3}", uinLabel, uiNode.type, uiNode.id, uiNode.sine.input);
                break;
            }
            case UiNodeType::time:
                LOG_TRACE("{0} {1} {2}", uinLabel, uiNode.type, uiNode.id);
                break;
            default:
                LOG_ERROR("Invalid UiNode::UiNodeType {0}", static_cast<int>(uiNode.type));
                break;
            }
        }

        fin.close();
    }


    enum class UiNodeType
    {
        add,
        multiply,
        output,
        sine,
        time,
    };

private:
    

    struct UiNode
    {
        UiNodeType type;
        // The identifying id of the ui node. For add, multiply, sine, and time
        // this is the "operation" node id. The additional input nodes are
        // stored in the structs.
        int id;

        union
        {
            struct
            {
                int lhs, rhs;
            } add;

            struct
            {
                int lhs, rhs;
            } multiply;

            struct
            {
                int r, g, b;
            } output;

            struct
            {
                int input;
            } sine;
        };

        friend std::ostream& operator<<(std::ostream& out, const UiNode& n)
        {
            out << static_cast<int>(n.type)
                << " "
                << n.id;

            switch (n.type)
            {
            case UiNodeType::add:
            {
                out << " "
                    << n.add.lhs
                    << " "
                    << n.add.rhs;
                break;
            }
            case UiNodeType::multiply:
            {
                out << " "
                    << n.multiply.lhs
                    << " "
                    << n.multiply.rhs;
                break;
            }
            case UiNodeType::output:
            {
                out << " "
                    << n.output.r
                    << " "
                    << n.output.g
                    << " "
                    << n.output.b;
                break;
            }
            case UiNodeType::sine:
            {
                out << " "
                    << n.sine.input;
                break;
            }
            case UiNodeType::time:
                break;
            default:
                LOG_ERROR("Invalid UiNode::UiNodeType {0}", static_cast<int>(n.type));
                break;
            }
            
            return out;
        }
    };

    Graph<Node>         graph_;
    std::vector<UiNode> uiNodes_;
    int                 root_node_id_;
};

static ColorNodeEditor color_editor;