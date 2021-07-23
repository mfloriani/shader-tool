#pragma once

#include "UiNode.h"

struct AddNode : UiNode
{
    explicit AddNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Left(INVALID_ID), Right(INVALID_ID)
    {
    }

    explicit AddNode(UiNodeType type, UiNodeId id, NodeId lhs, NodeId rhs)
        : UiNode(type, id), Left(lhs), Right(rhs)
    {
    }

    NodeId Left, Right;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Left);
        graph.EraseNode(Right);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("ADD");
        ImNodes::EndNodeTitleBar();
        {
            ImNodes::BeginInputAttribute(Left);
            const float label_width = ImGui::CalcTextSize("left").x;
            ImGui::TextUnformatted("left");
            if (graph.GetNumEdgesFromNode(Left) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(Left).Value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginInputAttribute(Right);
            const float label_width = ImGui::CalcTextSize("right").x;
            ImGui::TextUnformatted("right");
            if (graph.GetNumEdgesFromNode(Right) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(Right).Value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginOutputAttribute(Id);
            const float label_width = ImGui::CalcTextSize("result").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("result");
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Left << " " << Right;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Left >> Right;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const AddNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, AddNode& n)
    {
        return n.Deserialize(in);
    }
};