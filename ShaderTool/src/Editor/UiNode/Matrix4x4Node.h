#pragma once

#include "UiNode.h"
#include "Editor/ImGui/imgui_internal.h"

struct Matrix4x4Node : UiNode
{
private:

public:
    NodeId InputPin, OutputPin;
    std::shared_ptr<NodeValue<DirectX::XMFLOAT4X4>> InputNodeValue;
    std::shared_ptr<NodeValue<DirectX::XMFLOAT4X4>> OutputNodeValue;

public:
    explicit Matrix4x4Node(Graph* graph)
        : UiNode(graph, UiNodeType::Matrix4x4), InputPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        InputNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT4X4>>();
        InputNodeValue->TypeName = "float4x4";
        InputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[InputNodeValue->TypeName];
        InputNodeValue->Data = D3DUtil::Identity4x4();

        OutputNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT4X4>>();
        OutputNodeValue->TypeName = "float4x4";
        OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = D3DUtil::Identity4x4();
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Matrix4x4, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node inputNode(NodeType::Float4x4, NodeDirection::In);
        InputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, InputPin, EdgeType::Internal);
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(InputPin, InputNodeValue);

        const Node outputNode(NodeType::Float4x4, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(InputPin, InputNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {
        CopyFromLinkedSourceNodeValue<DirectX::XMFLOAT4X4>(InputPin, InputNodeValue->Data);
    }

    virtual void OnEval() override
    {
        OutputNodeValue->Data = InputNodeValue->Data;
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
            ImGui::DragFloat4("##hidelabel0", &InputNodeValue->Data._11, 0.01f);
            ImGui::DragFloat4("##hidelabel1", &InputNodeValue->Data._21, 0.01f);
            ImGui::DragFloat4("##hidelabel2", &InputNodeValue->Data._31, 0.01f);
            ImGui::DragFloat4("##hidelabel3", &InputNodeValue->Data._41, 0.01f);
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
        out << " " << InputPin
            << " " << OutputPin
            << " " << InputNodeValue->Data._11
            << " " << InputNodeValue->Data._12
            << " " << InputNodeValue->Data._13
            << " " << InputNodeValue->Data._14
            << " " << InputNodeValue->Data._21
            << " " << InputNodeValue->Data._22
            << " " << InputNodeValue->Data._23
            << " " << InputNodeValue->Data._24
            << " " << InputNodeValue->Data._31
            << " " << InputNodeValue->Data._32
            << " " << InputNodeValue->Data._33
            << " " << InputNodeValue->Data._34
            << " " << InputNodeValue->Data._41
            << " " << InputNodeValue->Data._42
            << " " << InputNodeValue->Data._43
            << " " << InputNodeValue->Data._44;

        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Matrix4x4;
        in >> Id
            >> InputPin
            >> OutputPin
            >> InputNodeValue->Data._11
            >> InputNodeValue->Data._12
            >> InputNodeValue->Data._13
            >> InputNodeValue->Data._14
            >> InputNodeValue->Data._21
            >> InputNodeValue->Data._22
            >> InputNodeValue->Data._23
            >> InputNodeValue->Data._24
            >> InputNodeValue->Data._31
            >> InputNodeValue->Data._32
            >> InputNodeValue->Data._33
            >> InputNodeValue->Data._34
            >> InputNodeValue->Data._41
            >> InputNodeValue->Data._42
            >> InputNodeValue->Data._43
            >> InputNodeValue->Data._44;
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