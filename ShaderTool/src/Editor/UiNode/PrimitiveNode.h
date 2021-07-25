#pragma once

#include "UiNode.h"

struct PrimitiveNode : UiNode
{
    explicit PrimitiveNode(Graph<Node>* graph, std::vector<const char*>& primitives)
        : UiNode(graph, UiNodeType::Primitive), Primitives(primitives), Model(0)
    {
    }

    std::vector<const char*>& Primitives;
    int Model;

    virtual void OnCreate() override
    {
        const Node op(NodeType::Primitive);
        Id = ParentGraph->CreateNode(op);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
    }

    virtual void OnDelete() override
    {
        
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("PRIMITIVE");
        ImNodes::EndNodeTitleBar();

        ImGui::PushItemWidth(node_width);
        
        Model = static_cast<int>(ParentGraph->GetNode(Id).Value);
        ImGui::Combo("##hidelabel", &Model, Primitives.data(), (int)Primitives.size());
        ImGui::PopItemWidth();
        ParentGraph->GetNode(Id).Value = static_cast<float>(Model);

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