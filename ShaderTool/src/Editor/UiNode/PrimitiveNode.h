#pragma once

#include "UiNode.h"

struct PrimitiveNode : UiNode
{
    explicit PrimitiveNode(Graph<Node>* graph, std::vector<const char*>& primitives)
        : UiNode(graph, UiNodeType::Primitive), Primitives(primitives), SelectedModel(0)
    {
    }

    std::vector<const char*>& Primitives;
    int SelectedModel;

    virtual void OnCreate() override
    {
        const Node output(NodeType::Primitive);
        Id = ParentGraph->CreateNode(output);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        ParentGraph->GetNode(Id).Value = static_cast<float>(SelectedModel);
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
        ImGui::Combo("##hidelabel", &SelectedModel, Primitives.data(), (int)Primitives.size());
        ImGui::PopItemWidth();
        
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
        out << " " << SelectedModel;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Primitive;
        in >> Id >> SelectedModel;
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