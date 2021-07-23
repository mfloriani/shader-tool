#pragma once

#include "UiNode.h"

struct PrimitiveNode : UiNode
{
    explicit PrimitiveNode(Graph<Node>* graph, std::vector<const char*>& primitives)
        : UiNode(graph, UiNodeType::Primitive), Primitives(primitives), Model(INVALID_ID)
    {
    }

    std::vector<const char*>& Primitives;
    NodeId Model;

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node op(NodeType::Primitive);

        Model = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(op);

        ParentGraph->CreateEdge(Id, Model);
    }

    virtual void OnUpdate() override
    {
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(Model);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("PRIMITIVE");
        ImNodes::EndNodeTitleBar();

        ImGui::PushItemWidth(node_width);
        auto& modelNode = ParentGraph->GetNode(Model);
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
        Type = UiNodeType::Primitive;
        in >> Id >> Model;
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