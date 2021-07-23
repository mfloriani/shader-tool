#pragma once

#include "UiNode.h"

struct SineNode : UiNode
{
    explicit SineNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Input(INVALID_ID)
    {
    }

    explicit SineNode(UiNodeType type, UiNodeId id, NodeId input)
        : UiNode(type, id), Input(input)
    {
    }

    NodeId Input;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Input);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("SINE");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(Input);
            const float label_width = ImGui::CalcTextSize("number").x;
            ImGui::TextUnformatted("number");
            if (graph.GetNumEdgesFromNode(Input) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(Input).value, 0.01f, 0.f, 1.0f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginOutputAttribute(Id);
            const float label_width = ImGui::CalcTextSize("output").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("output");
            ImNodes::EndInputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Input;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Input;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const SineNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, SineNode& n)
    {
        return n.Deserialize(in);
    }
};