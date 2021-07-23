#pragma once

#include "UiNode.h"

struct AddNode : UiNode
{
    explicit AddNode(Graph<Node>* graph)
        : UiNode(graph, UiNodeType::Add), Left(INVALID_ID), Right(INVALID_ID)
    {
    }

    NodeId Left, Right;

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node op(NodeType::Add);

        Left = ParentGraph->CreateNode(value);
        Right = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(op);

        ParentGraph->CreateEdge(Id, Left);
        ParentGraph->CreateEdge(Id, Right);
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(Left);
        ParentGraph->EraseNode(Right);
    }

    virtual void OnUpdate() override
    {
    }

    virtual void OnRender() override
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
            if (ParentGraph->GetNumEdgesFromNode(Left) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &ParentGraph->GetNode(Left).Value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginInputAttribute(Right);
            const float label_width = ImGui::CalcTextSize("right").x;
            ImGui::TextUnformatted("right");
            if (ParentGraph->GetNumEdgesFromNode(Right) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &ParentGraph->GetNode(Right).Value, 0.01f);
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
        Type = UiNodeType::Add;
        in >> Id >> Left >> Right;
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