#pragma once

#include "UiNode.h"

struct ShaderNode : UiNode
{
private:
    std::string _Path;
    std::string _ShaderName;

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValue<int>> OutputNodeValue;

public:
    explicit ShaderNode(Graph* graph)
        : UiNode(graph, UiNodeType::Shader)
    {
        OutputNodeValue = std::make_shared<NodeValue<int>>();
        OutputNodeValue->TypeName = "int";
        OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = INVALID_INDEX;
    }

    const std::string GetPath() const { return _Path; }
    const std::string GetName() const { return _ShaderName; }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        _Path = "INTERNAL_SHADER_PATH";
        _ShaderName = DEFAULT_SHADER;
        OutputNodeValue->Data = (int)ShaderManager::Get().GetShaderIndex(_ShaderName);

        const Node idNode = Node(NodeType::Shader, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node shaderIndexNodeOut(NodeType::Int, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(shaderIndexNodeOut);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        
        StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        if (!ShaderManager::Get().HasShader(_ShaderName))
        {
            _ShaderName = ShaderManager::Get().LoadShaderFromFile(_Path);
            if (!ShaderManager::Get().HasShader(_ShaderName))
            {
                LOG_ERROR("Failed to load shader {0}! The default shader was used instead.", _Path);

                _Path = "INTERNAL_SHADER_PATH";
                _ShaderName = DEFAULT_SHADER;
            }
        }
        OutputNodeValue->Data = (int)ShaderManager::Get().GetShaderIndex(_ShaderName);
        StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {

    }
    
    virtual void OnEval() override
    {

    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;

        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("VERTEX SHADER [?]");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Expected function names:\nVS, HS, DS, GS and PS");
        ImNodes::EndNodeTitleBar();
        
        if (ImGui::Button("Open"))
        {
            nfdchar_t* outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("*", NULL, &outPath);

            if (result == NFD_OKAY)
            {
                _Path = std::string(outPath);
                _ShaderName = ShaderManager::Get().LoadShaderFromFile(_Path);
                OnLoad();
                free(outPath);
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
        //ImGui::SameLine();
        //if (ImGui::Button("Edit"))
        //{

        //}
        //ImGui::SameLine();
        //if (ImGui::Button("Compile"))
        //{

        //}

        ImGui::Text(_ShaderName.c_str());

        ImNodes::BeginOutputAttribute(OutputPin);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin << " " << _ShaderName << " " << _Path;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Shader;
        in >> Id >> OutputPin >> _ShaderName >> _Path;
        
        OnLoad();
                
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
