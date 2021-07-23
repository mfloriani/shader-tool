#pragma once

#include "UiNode.h"

struct TimeNode : UiNode
{
    explicit TimeNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id)
    {
    }

    virtual void Delete(Graph<Node>& graph) override
    {
    }

    virtual void Render(Graph<Node>& graph) override
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
