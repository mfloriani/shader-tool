#pragma once

#include "UiNode.h"

struct DrawNode : UiNode
{
    explicit DrawNode(Graph<Node>* graph)
        : UiNode(graph, UiNodeType::Draw), Model(INVALID_ID), Shader(INVALID_ID)
    {
    }

    NodeId Model, Shader;

    struct DrawData
    {
        DrawData() : model(NOT_LINKED), shader(NOT_LINKED), output(NOT_LINKED) {}
        int model, shader, output;
    } Data;

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node link(NodeType::Link, NOT_LINKED);
        const Node out(NodeType::Draw);

        Shader = ParentGraph->CreateNode(link);
        Model = ParentGraph->CreateNode(link);
        Id = ParentGraph->CreateNode(out);

        ParentGraph->CreateEdge(Id, Shader);
        ParentGraph->CreateEdge(Id, Model);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(Model);
        ParentGraph->EraseNode(Shader);
    }

    virtual void OnRender() override
    {
        const float node_width = 200.0f;
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

        // SHADER BINDINGS

        ImGui::Spacing();
        ImGui::Text("BINDS");
        ImGui::Spacing();

        if (Data.shader != NOT_LINKED)
        {
            int id = 100; // TODO: TEMPORARY
            auto vars = ShaderManager::Get().GetShader(Data.shader)->GetConstantBufferVars();
            for (auto& var : vars)
            {
                std::string varNameType = var.Name + " (" + var.Type.Name + ")";

                ImNodes::BeginInputAttribute(id++);
                const float label_width = ImGui::CalcTextSize(varNameType.c_str()).x;
                ImGui::TextUnformatted(varNameType.c_str());
                ImNodes::EndInputAttribute();
                ImGui::Spacing();
            }
        }

        ImGui::Spacing(); ImGui::Spacing();

        {
            ImNodes::BeginOutputAttribute(Id);
            const float label_width = ImGui::CalcTextSize("output").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("output");
            ImNodes::EndOutputAttribute();
        }

        ImGui::Spacing();
                
        ImNodes::EndNode();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Model << " " << Shader;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Draw;
        in >> Id >> Model >> Shader;
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