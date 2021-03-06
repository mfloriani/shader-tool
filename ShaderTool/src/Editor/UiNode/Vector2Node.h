#pragma once

#include "UiNode.h"

struct Vector2Node : UiNode
{
private:

public:
    NodeId XInputPin, YInputPin, OutputPin;

    std::shared_ptr<NodeValueFloat> XInputNodeValue;
    std::shared_ptr<NodeValueFloat> YInputNodeValue;
    std::shared_ptr<NodeValueFloat2> OutputNodeValue;

public:
    explicit Vector2Node(Graph* graph)
        : UiNode(graph, UiNodeType::Vector2),
        XInputPin(INVALID_ID), YInputPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        XInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        YInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        OutputNodeValue = std::make_shared<NodeValueFloat2>(DirectX::XMFLOAT2(0.f, 0.f));
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Vector2, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node inputNode(NodeType::Float, NodeDirection::In);

        XInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, XInputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(XInputPin, XInputNodeValue);

        YInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, YInputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(YInputPin, YInputNodeValue);

        const Node outputNode(NodeType::Float2, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        XInputNodeValue->Value = *(float*)ParentGraph->GetNodeValue(XInputPin)->GetValuePtr();
        YInputNodeValue->Value = *(float*)ParentGraph->GetNodeValue(YInputPin)->GetValuePtr();

        ParentGraph->StoreNodeValue(XInputPin, XInputNodeValue);
        ParentGraph->StoreNodeValue(YInputPin, YInputNodeValue);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }


    virtual void OnUpdate() override
    {
        CopyValueFromLinkedSource(XInputPin, (void*)&XInputNodeValue->Value);
        CopyValueFromLinkedSource(YInputPin, (void*)&YInputNodeValue->Value);
    }

    virtual void OnEval() override
    {
        OutputNodeValue->Value = DirectX::XMFLOAT2( XInputNodeValue->Value, YInputNodeValue->Value);
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(XInputPin);
        ParentGraph->EraseNode(YInputPin);
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("VECTOR2");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(XInputPin);
            const float label_width = ImGui::CalcTextSize("x").x;
            ImGui::TextUnformatted("x");
            if (ParentGraph->GetNumEdgesFromNode(XInputPin) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &XInputNodeValue->Value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginInputAttribute(YInputPin);
            const float label_width = ImGui::CalcTextSize("y").x;
            ImGui::TextUnformatted("y");
            if (ParentGraph->GetNumEdgesFromNode(YInputPin) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &YInputNodeValue->Value, 0.01f);
                ImGui::PopItemWidth();
            }
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
        out << " " << XInputPin
            << " " << YInputPin
            << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Vector2;
        in >> Id
            >> XInputPin
            >> YInputPin
            >> OutputPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector2Node& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, Vector2Node& n)
    {
        return n.Deserialize(in);
    }
};