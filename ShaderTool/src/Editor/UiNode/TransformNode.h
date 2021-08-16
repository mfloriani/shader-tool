#pragma once

#include "UiNode.h"

struct TransformNode : UiNode
{
private:
    DirectX::XMFLOAT3 _EyePos = { 0.0f, 0.0f, 0.0f };
    

public:
    NodeId PositionPin, RotationPin, ScalePin, OutputPin;
    std::shared_ptr<NodeValueFloat3> PositionNodeValue;
    std::shared_ptr<NodeValueFloat3> RotationNodeValue;
    std::shared_ptr<NodeValueFloat3> ScaleNodeValue;
    std::shared_ptr<NodeValueFloat4x4> OutputNodeValue;

public:
    explicit TransformNode(Graph* graph)
        : UiNode(graph, UiNodeType::Transform), 
        PositionPin(INVALID_ID), RotationPin(INVALID_ID), ScalePin(INVALID_ID), OutputPin(INVALID_ID)
    {
        PositionNodeValue = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3(0.f, 0.f, 0.f));
        RotationNodeValue = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3(0.f, 0.f, 0.f));
        ScaleNodeValue = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3(1.f, 1.f, 1.f));
        OutputNodeValue = std::make_shared<NodeValueFloat4x4>(D3DUtil::Identity4x4());
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Transform, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node float3NodeIn(NodeType::Float3, NodeDirection::In);
        PositionPin = ParentGraph->CreateNode(float3NodeIn);
        RotationPin = ParentGraph->CreateNode(float3NodeIn);
        ScalePin = ParentGraph->CreateNode(float3NodeIn);

        const Node outputNode(NodeType::Float4x4, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);

        ParentGraph->CreateEdge(Id, PositionPin, EdgeType::Internal);
        ParentGraph->CreateEdge(Id, RotationPin, EdgeType::Internal);
        ParentGraph->CreateEdge(Id, ScalePin, EdgeType::Internal);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);

        ParentGraph->StoreNodeValue(PositionPin, PositionNodeValue);
        ParentGraph->StoreNodeValue(RotationPin, RotationNodeValue);
        ParentGraph->StoreNodeValue(ScalePin, ScaleNodeValue);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        ParentGraph->StoreNodeValue(PositionPin, PositionNodeValue);
        ParentGraph->StoreNodeValue(RotationPin, RotationNodeValue);
        ParentGraph->StoreNodeValue(ScalePin, ScaleNodeValue);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {
        CopyValueFromLinkedSource(PositionPin, (void*)&PositionNodeValue->Value);
        CopyValueFromLinkedSource(RotationPin, (void*)&RotationNodeValue->Value);
        CopyValueFromLinkedSource(ScalePin, (void*)&ScaleNodeValue->Value);
    }

    virtual void OnEval() override
    {
        auto position = DirectX::XMMatrixTranslation(PositionNodeValue->Value.x, PositionNodeValue->Value.y, PositionNodeValue->Value.z);
        
        auto xRot = DirectX::XMMatrixRotationX(RotationNodeValue->Value.x);
        auto yRot = DirectX::XMMatrixRotationY(RotationNodeValue->Value.y);
        auto zRot = DirectX::XMMatrixRotationZ(RotationNodeValue->Value.z);
        auto rotation = DirectX::XMMatrixMultiply(xRot, DirectX::XMMatrixMultiply(yRot, zRot));

        auto scale = DirectX::XMMatrixScaling(ScaleNodeValue->Value.x, ScaleNodeValue->Value.y, ScaleNodeValue->Value.z);

        auto world = DirectX::XMMatrixMultiply(scale, DirectX::XMMatrixMultiply(rotation, position));
        DirectX::XMStoreFloat4x4(&OutputNodeValue->Value, DirectX::XMMatrixTranspose(world));
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(PositionPin);
        ParentGraph->EraseNode(RotationPin);
        ParentGraph->EraseNode(ScalePin);
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 170.0f;

        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("TRANSFORM");
        ImNodes::EndNodeTitleBar();

        {
            bool hasLink = ParentGraph->GetNumEdgesFromNode(PositionPin) > 0ull;
            if (hasLink) ImGui::PushDisabled();
            
            ImNodes::BeginInputAttribute(PositionPin);
            ImGui::PushItemWidth(node_width);
            ImGui::TextUnformatted("position"); ImGui::SameLine();
            ImGui::DragFloat3("##hidelabel0", &PositionNodeValue->Value.x, 0.01f);
            ImGui::PopItemWidth();
            ImNodes::EndInputAttribute();

            if (hasLink) ImGui::PopDisabled();
        }

        ImNodes::BeginInputAttribute(RotationPin);
        ImGui::PushItemWidth(node_width);
        ImGui::TextUnformatted("rotation"); ImGui::SameLine();
        ImGui::DragFloat3("##hidelabel1", &RotationNodeValue->Value.x, 0.01f);
        ImGui::PopItemWidth();
        ImNodes::EndInputAttribute();

        ImNodes::BeginInputAttribute(ScalePin);
        ImGui::PushItemWidth(node_width);
        ImGui::TextUnformatted("scale   "); ImGui::SameLine();
        ImGui::DragFloat3("##hidelabel2", &ScaleNodeValue->Value.x, 0.01f);
        ImGui::PopItemWidth();
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(OutputPin);
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << PositionPin << " " << RotationPin << " " << ScalePin << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Transform;
        in >> Id >> PositionPin >> RotationPin >> ScalePin >> OutputPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const TransformNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, TransformNode& n)
    {
        return n.Deserialize(in);
    }
};