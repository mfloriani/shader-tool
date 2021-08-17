#pragma once

#include "UiNode.h"

struct ShaderNode : UiNode
{
private:

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValueInt> OutputNodeValue;

public:
    explicit ShaderNode(Graph* graph) 
        : UiNode(graph, UiNodeType::Shader), OutputPin(INVALID_ID)
    {
        OutputNodeValue = std::make_shared<NodeValueInt>(INVALID_INDEX);
    }

    const std::string GetPath() const 
    {
        return ShaderManager::Get()->GetShaderPath(OutputNodeValue->Value);
    }
    
    const std::string GetName() const 
    {
        return ShaderManager::Get()->GetShaderName(OutputNodeValue->Value);
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        OutputNodeValue->Value = (int)ShaderManager::Get()->GetShaderIndex(DEFAULT_SHADER);

        const Node idNode = Node(NodeType::Shader, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node shaderIndexNodeOut(NodeType::Int, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(shaderIndexNodeOut);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        OutputNodeValue->Value = *(int*)ParentGraph->GetNodeValue(OutputPin)->GetValuePtr();
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
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
                OutputNodeValue->Value = ShaderManager::Get()->LoadShaderFromFile(std::string(outPath));
                EVENT_MANAGER.Enqueue(std::make_shared<ShaderUpdatedEvent>(OutputNodeValue->Value));
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

        ImGui::Text(GetName().c_str());

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
        out << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Shader;
        in >> Id >> OutputPin;
        
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
