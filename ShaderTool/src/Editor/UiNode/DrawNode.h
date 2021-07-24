#pragma once

#include "UiNode.h"

struct DrawNode : UiNode
{
    explicit DrawNode(Graph<Node>* graph)
        : UiNode(graph, UiNodeType::Draw), R(INVALID_ID), G(INVALID_ID), B(INVALID_ID), Model(INVALID_ID), VS(INVALID_ID), PS(INVALID_ID)
    {
    }

    NodeId R, G, B, Model, VS, PS;

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node out(NodeType::Draw);

        VS = ParentGraph->CreateNode(value);
        PS = ParentGraph->CreateNode(value);
        Model = ParentGraph->CreateNode(value);
        R = ParentGraph->CreateNode(value);
        G = ParentGraph->CreateNode(value);
        B = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(out);

        ParentGraph->CreateEdge(Id, VS);
        ParentGraph->CreateEdge(Id, PS);
        ParentGraph->CreateEdge(Id, Model);
        ParentGraph->CreateEdge(Id, R);
        ParentGraph->CreateEdge(Id, G);
        ParentGraph->CreateEdge(Id, B);
    }

    virtual void OnUpdate() override
    {
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(R);
        ParentGraph->EraseNode(G);
        ParentGraph->EraseNode(B);
        ParentGraph->EraseNode(Model);
        ParentGraph->EraseNode(VS);
        ParentGraph->EraseNode(PS);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(81, 148, 204, 255));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(81, 148, 204, 255));
        //ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(255, 255, 0, 255));
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DRAW");
        ImNodes::EndNodeTitleBar();

        ImGui::Dummy(ImVec2(node_width, 0.f));

        {
            ImNodes::BeginInputAttribute(VS);
            const float label_width = ImGui::CalcTextSize("vs").x;
            ImGui::TextUnformatted("vs");
            if (ParentGraph->GetNumEdgesFromNode(VS) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginInputAttribute(PS);
            const float label_width = ImGui::CalcTextSize("ps").x;
            ImGui::TextUnformatted("ps");
            if (ParentGraph->GetNumEdgesFromNode(PS) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginInputAttribute(Model);
            const float label_width = ImGui::CalcTextSize("model").x;
            ImGui::TextUnformatted("model");
            if (ParentGraph->GetNumEdgesFromNode(Model) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginInputAttribute(R);
            const float label_width = ImGui::CalcTextSize("r").x;
            ImGui::TextUnformatted("r");
            if (ParentGraph->GetNumEdgesFromNode(R) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &ParentGraph->GetNode(R).Value, 0.01f, 0.f, 1.0f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginInputAttribute(G);
            const float label_width = ImGui::CalcTextSize("g").x;
            ImGui::TextUnformatted("g");
            if (ParentGraph->GetNumEdgesFromNode(G) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &ParentGraph->GetNode(G).Value, 0.01f, 0.f, 1.f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginInputAttribute(B);
            const float label_width = ImGui::CalcTextSize("b").x;
            ImGui::TextUnformatted("b");
            if (ParentGraph->GetNumEdgesFromNode(B) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &ParentGraph->GetNode(B).Value, 0.01f, 0.f, 1.0f);
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
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << R << " " << G << " " << B << " " << Model << " " << VS << " " << PS;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Draw;
        in >> Id >> R >> G >> B >> Model >> VS >> PS;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const DrawNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, DrawNode& n)
    {
        return n.Deserialize(in);
    }
};