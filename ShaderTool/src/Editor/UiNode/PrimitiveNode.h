#pragma once

#include "UiNode.h"

struct PrimitiveNode : UiNode
{
    explicit PrimitiveNode(UiNodeType type, UiNodeId id, std::vector<const char*>& primitives)
        : UiNode(type, id), Primitives(primitives), Model(INVALID_ID)
    {
    }

    explicit PrimitiveNode(UiNodeType type, UiNodeId id, std::vector<const char*>& primitives, NodeId model)
        : UiNode(type, id), Primitives(primitives), Model(model)
    {
    }

    std::vector<const char*>& Primitives;
    NodeId Model;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Model);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("PRIMITIVE");
        ImNodes::EndNodeTitleBar();

        ImGui::PushItemWidth(node_width);
        auto& modelNode = graph.GetNode(Model);
        int value = static_cast<int>(modelNode.Value);
        ImGui::Combo("##hidelabel", &value, Primitives.data(), (int)Primitives.size());
        ImGui::PopItemWidth();
        modelNode.Value = static_cast<float>(value);

        ImNodes::BeginOutputAttribute(Id);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Model;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Model;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const PrimitiveNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, PrimitiveNode& n)
    {
        return n.Deserialize(in);
    }
};