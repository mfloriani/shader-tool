#pragma once

#include "UiNode.h"

struct PixelShaderNode : UiNode
{
    explicit PixelShaderNode(Graph<Node>* graph)
        : UiNode(graph, UiNodeType::PixelShader), Input(INVALID_ID)
    {
    }

    NodeId Input;
    std::string Path;

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node op(NodeType::PixelShader);

        Input = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(op);

        ParentGraph->CreateEdge(Id, Input);
    }

    virtual void OnUpdate() override
    {
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(Input);
    }

    virtual void OnRender() override
    {
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("PIXEL SHADER");
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
                int shaderIndex = ShaderManager::Get().LoadRawShader(shaderPath, "main", "ps_5_0");
                if (shaderIndex < 0)
                {
                    // Failed
                    LOG_WARN("### Failed to load the selected shader! The previous shader was kept.");
                }
                else
                {
                    // Loaded and Compiled successfully
                    Path = shaderPath;
                    Input = shaderIndex;
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

        ImNodes::BeginOutputAttribute(Id);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

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
        Type = UiNodeType::PixelShader;
        in >> Id >> Input;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const PixelShaderNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, PixelShaderNode& n)
    {
        return n.Deserialize(in);
    }
};
