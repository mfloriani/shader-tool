#pragma once

#include "UiNode.h"

struct DrawNode : UiNode
{
    explicit DrawNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), R(INVALID_ID), G(INVALID_ID), B(INVALID_ID), Model(INVALID_ID), VS(INVALID_ID), PS(INVALID_ID)
    {
    }

    explicit DrawNode(UiNodeType type, UiNodeId id, NodeId r, NodeId g, NodeId b, NodeId model, NodeId vs, NodeId ps)
        : UiNode(type, id), R(r), G(g), B(b), Model(model), VS(vs), PS(ps)
    {
    }

    NodeId R, G, B, Model, VS, PS;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(R);
        graph.EraseNode(G);
        graph.EraseNode(B);
        graph.EraseNode(Model);
        graph.EraseNode(VS);
        graph.EraseNode(PS);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(45, 126, 194, 255));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(81, 148, 204, 255));
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DRAW");
        ImNodes::EndNodeTitleBar();

        ImGui::Dummy(ImVec2(node_width, 0.f));

        {
            ImNodes::BeginInputAttribute(VS);
            const float label_width = ImGui::CalcTextSize("vs").x;
            ImGui::TextUnformatted("vs");
            if (graph.GetNumEdgesFromNode(VS) == 0ull)
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
            if (graph.GetNumEdgesFromNode(PS) == 0ull)
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
            if (graph.GetNumEdgesFromNode(Model) == 0ull)
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
            if (graph.GetNumEdgesFromNode(R) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(R).Value, 0.01f, 0.f, 1.0f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginInputAttribute(G);
            const float label_width = ImGui::CalcTextSize("g").x;
            ImGui::TextUnformatted("g");
            if (graph.GetNumEdgesFromNode(G) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(G).Value, 0.01f, 0.f, 1.f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginInputAttribute(B);
            const float label_width = ImGui::CalcTextSize("b").x;
            ImGui::TextUnformatted("b");
            if (graph.GetNumEdgesFromNode(B) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(B).Value, 0.01f, 0.f, 1.0f);
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
        in >> R >> G >> B >> Model >> VS >> PS;
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