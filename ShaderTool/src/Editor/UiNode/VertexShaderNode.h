#pragma once

#include "UiNode.h"

struct VertexShaderNode : UiNode
{
    explicit VertexShaderNode(Graph<Node>* graph)
        : UiNode(graph, UiNodeType::VertexShader)
    {
    }

    struct VSData
    {
        std::string path;
        std::string shaderName;
        int shaderIndex{ INVALID_INDEX };
    } Data;

    virtual void OnCreate() override
    {
        const Node op(NodeType::VertexShader);
        Id = ParentGraph->CreateNode(op);

        Data.path = "INTERNAL_SHADER_PATH";
        Data.shaderName = DEFAULT_VS;
        Data.shaderIndex = (int)ShaderManager::Get().GetShaderIndex(Data.shaderName);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        ParentGraph->GetNode(Id).Value = static_cast<float>(Data.shaderIndex);
    }

    virtual void OnDelete() override
    {
        
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;

        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("VERTEX SHADER");
        ImNodes::EndNodeTitleBar();

        if (ImGui::Button("..."))
        {
            nfdchar_t* outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("hlsl", NULL, &outPath);

            if (result == NFD_OKAY)
            {
                auto shaderPath = std::string(outPath);
                free(outPath);
                //LOG_TRACE("Selected file [{0} | {1} | {2}]", path, D3DUtil::ExtractFilename(path, true), D3DUtil::ExtractFilename(path));
                auto shaderName = ShaderManager::Get().LoadRawShader(shaderPath, "main", "vs_5_0");
                if (shaderName.size() == 0)
                {
                    // Failed
                    LOG_WARN("### Failed to load the selected shader! The previous shader was kept.");
                }
                else
                {
                    // Loaded and Compiled successfully
                    Data.path = shaderPath;
                    Data.shaderName = shaderName;
                    Data.shaderIndex = (int) ShaderManager::Get().GetShaderIndex(Data.shaderName);
                }
            }
            else if (result == NFD_CANCEL)
            {
                LOG_TRACE("User pressed cancel.");
            }
            else
            {
                LOG_ERROR("Error: %s\n", NFD_GetError());
            }
        }
        
        ImGui::Text(Data.shaderName.c_str());

        ImNodes::BeginOutputAttribute(Id);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Data.shaderName << " " << Data.path;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::VertexShader;
        in >> Id >> Data.shaderName >> Data.path;
        Data.shaderIndex = (int) ShaderManager::Get().GetShaderIndex(Data.shaderName);
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const VertexShaderNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, VertexShaderNode& n)
    {
        return n.Deserialize(in);
    }
};
