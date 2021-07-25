#pragma once

#include "UiNode.h"

struct TimeNode : UiNode
{
    explicit TimeNode(Graph<Node>* graph)
        : UiNode(graph, UiNodeType::Time)
    {
    }

    virtual void OnCreate() override
    {
        Id = ParentGraph->CreateNode(Node(NodeType::Time));
    }

    virtual void OnDelete() override
    {
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        ParentGraph->GetNode(Id).Value = timer.TotalTime();
    }

    virtual void OnRender() override
    {
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("TIME");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginOutputAttribute(Id);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Time;
        in >> Id;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const TimeNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, TimeNode& n)
    {
        return n.Deserialize(in);
    }
};
