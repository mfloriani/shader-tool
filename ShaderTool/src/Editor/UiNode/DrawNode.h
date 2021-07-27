#pragma once

#include "UiNode.h"

struct DrawNode : UiNode
{
    explicit DrawNode(Graph<Node>* graph)
        : UiNode(graph, UiNodeType::Draw), R(INVALID_ID), G(INVALID_ID), B(INVALID_ID), Model(INVALID_ID), Shader(INVALID_ID)
    {
    }

    NodeId R, G, B, Model, Shader;

    struct DrawData
    {
        DrawData() : r(0.f), g(0.f), b(0.f), model(NOT_LINKED), shader(NOT_LINKED), output(NOT_LINKED) {}
        float r, g, b;
        int model, shader, output;
    } Data;

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node link(NodeType::Link, NOT_LINKED);
        const Node out(NodeType::Draw);

        Shader = ParentGraph->CreateNode(link);
        Model = ParentGraph->CreateNode(link);
        R = ParentGraph->CreateNode(value);
        G = ParentGraph->CreateNode(value);
        B = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(out);

        ParentGraph->CreateEdge(Id, Shader);
        ParentGraph->CreateEdge(Id, Model);
        ParentGraph->CreateEdge(Id, R);
        ParentGraph->CreateEdge(Id, G);
        ParentGraph->CreateEdge(Id, B);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        //Data.vs = (int)ParentGraph->GetNode(VS).Value;
        //Data.ps = (int)ParentGraph->GetNode(PS).Value;
        //Data.model = (int)ParentGraph->GetNode(Model).Value;
        //Data.r = ParentGraph->GetNode(R).Value;
        //Data.g = ParentGraph->GetNode(G).Value;
        //Data.b = ParentGraph->GetNode(B).Value;
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(R);
        ParentGraph->EraseNode(G);
        ParentGraph->EraseNode(B);
        ParentGraph->EraseNode(Model);
        ParentGraph->EraseNode(Shader);
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
            ImNodes::BeginInputAttribute(Shader);
            const float label_width = ImGui::CalcTextSize("shader").x;
            ImGui::TextUnformatted("shader");
            if (ParentGraph->GetNumEdgesFromNode(Shader) == 0ull)
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
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << R << " " << G << " " << B << " " << Model << " " << Shader;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Draw;
        in >> Id >> R >> G >> B >> Model >> Shader;
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