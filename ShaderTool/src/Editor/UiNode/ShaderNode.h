#pragma once

#include "UiNode.h"

struct ShaderNode : UiNode
{
    explicit ShaderNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Shader(INVALID_ID)
    {
        Init();
    }

    explicit ShaderNode(UiNodeType type, UiNodeId id, NodeId shader)
        : UiNode(type, id), Shader(shader)
    {
        Init();
    }

    NodeId Shader;
    std::string Path;

    void Init()
    {
        
    }

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Shader);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        switch (Type)
        {
        case UiNodeType::VertexShader:
            ImGui::TextUnformatted("VERTEX SHADER");
            break;
        case UiNodeType::PixelShader:
            ImGui::TextUnformatted("PIXEL SHADER");
            break;
        default:
            ImGui::TextUnformatted("UNKNOWN SHADER");
            break;
        }
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
                int shaderIndex = ShaderManager::Get().LoadRawShader(shaderPath, "main", "vs_5_0");
                if (shaderIndex < 0)
                {
                    // Failed
                    LOG_WARN("### Failed to load the selected shader! The previous shader was kept.");
                }
                else
                {
                    // Loaded and Compiled successfully
                    Path = shaderPath;
                    Shader = shaderIndex;
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
        out << " " << Shader;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Shader;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const ShaderNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, ShaderNode& n)
    {
        return n.Deserialize(in);
    }
};
