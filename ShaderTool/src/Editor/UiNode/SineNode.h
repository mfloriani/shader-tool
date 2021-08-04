#pragma once

#include "UiNode.h"

struct SineNode : UiNode
{
    explicit SineNode(Graph* graph)
        : UiNode(graph, UiNodeType::Sine), Input(INVALID_ID)
    {
    }

    NodeId Input;

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node op(NodeType::Sine);

        Input = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(op);

        ParentGraph->CreateEdge(Id, Input);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        //float input = ParentGraph->GetNode(Input).Value;
        //ParentGraph->GetNode(Id).Value = std::abs(std::sin(input));
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(Input);
    }

    virtual void OnRender() override
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
            if (ParentGraph->GetNumEdgesFromNode(Input) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &ParentGraph->GetNode(Input).Value, 0.01f, 0.f, 1.0f);
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
        Type = UiNodeType::Sine;
        in >> Id >> Input;
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