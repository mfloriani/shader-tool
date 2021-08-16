#pragma once

#include "UiNode.h"
#include "Editor/ImGui/imgui_internal.h"

struct Matrix4x4Node : UiNode
{
private:

public:
    NodeId InputPin, OutputPin;
    std::shared_ptr<NodeValueFloat4x4> InputNodeValue;
    std::shared_ptr<NodeValueFloat4x4> OutputNodeValue;

public:
    explicit Matrix4x4Node(Graph* graph)
        : UiNode(graph, UiNodeType::Matrix4x4), InputPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        InputNodeValue = std::make_shared<NodeValueFloat4x4>(D3DUtil::Identity4x4());
        OutputNodeValue = std::make_shared<NodeValueFloat4x4>(D3DUtil::Identity4x4());
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Matrix4x4, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node inputNode(NodeType::Float4x4, NodeDirection::In);
        InputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, InputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(InputPin, InputNodeValue);

        const Node outputNode(NodeType::Float4x4, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        InputNodeValue->Value = *(DirectX::XMFLOAT4X4*)ParentGraph->GetNodeValue(InputPin)->GetValuePtr();

        ParentGraph->StoreNodeValue(InputPin, InputNodeValue);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {
        CopyValueFromLinkedSource(InputPin, (void*)&InputNodeValue->Value);
    }

    virtual void OnEval() override
    {
        OutputNodeValue->Value = InputNodeValue->Value;
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(InputPin);
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 200.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("MATRIX4X4");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(InputPin);
            
            bool hasLink = ParentGraph->GetNumEdgesFromNode(InputPin) > 0ull;
            if (hasLink) ImGui::PushDisabled();

            ImGui::PushItemWidth(node_width);
            ImGui::DragFloat4("##hidelabel0", &InputNodeValue->Value._11, 0.01f);
            ImGui::DragFloat4("##hidelabel1", &InputNodeValue->Value._21, 0.01f);
            ImGui::DragFloat4("##hidelabel2", &InputNodeValue->Value._31, 0.01f);
            ImGui::DragFloat4("##hidelabel3", &InputNodeValue->Value._41, 0.01f);
            ImGui::PopItemWidth();
            
           if (hasLink) ImGui::PopDisabled();

            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginOutputAttribute(OutputPin);
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << InputPin << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Matrix4x4;
        in >> Id >> InputPin >> OutputPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Matrix4x4Node& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, Matrix4x4Node& n)
    {
        return n.Deserialize(in);
    }
};