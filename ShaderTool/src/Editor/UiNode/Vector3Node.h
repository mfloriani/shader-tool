#pragma once

#include "UiNode.h"

struct Vector3Node : UiNode
{
private:

public:
    NodeId XInputPin, YInputPin, ZInputPin, OutputPin;

    std::shared_ptr<NodeValueFloat> XInputNodeValue;
    std::shared_ptr<NodeValueFloat> YInputNodeValue;
    std::shared_ptr<NodeValueFloat> ZInputNodeValue;
    std::shared_ptr<NodeValueFloat3> OutputNodeValue;

public:
    explicit Vector3Node(Graph* graph)
        : UiNode(graph, UiNodeType::Vector3),
        XInputPin(INVALID_ID), YInputPin(INVALID_ID), ZInputPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        XInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        YInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        ZInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        OutputNodeValue = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3(0.f, 0.f, 0.f));
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Vector3, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node inputNode(NodeType::Float, NodeDirection::In);

        XInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, XInputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(XInputPin, XInputNodeValue);

        YInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, YInputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(YInputPin, YInputNodeValue);

        ZInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, ZInputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(ZInputPin, ZInputNodeValue);

        const Node outputNode(NodeType::Float3, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        XInputNodeValue->Value = *(float*)ParentGraph->GetNodeValue(XInputPin)->GetValuePtr();
        YInputNodeValue->Value = *(float*)ParentGraph->GetNodeValue(YInputPin)->GetValuePtr();
        ZInputNodeValue->Value = *(float*)ParentGraph->GetNodeValue(ZInputPin)->GetValuePtr();

        ParentGraph->StoreNodeValue(XInputPin, XInputNodeValue);
        ParentGraph->StoreNodeValue(YInputPin, YInputNodeValue);
        ParentGraph->StoreNodeValue(ZInputPin, ZInputNodeValue);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {
        CopyValueFromLinkedSource(XInputPin, (void*)&XInputNodeValue->Value);
        CopyValueFromLinkedSource(YInputPin, (void*)&YInputNodeValue->Value);
        CopyValueFromLinkedSource(ZInputPin, (void*)&ZInputNodeValue->Value);
    }

    virtual void OnEval() override
    {
        OutputNodeValue->Value = DirectX::XMFLOAT3(
            XInputNodeValue->Value,
            YInputNodeValue->Value,
            ZInputNodeValue->Value);
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(XInputPin);
        ParentGraph->EraseNode(YInputPin);
        ParentGraph->EraseNode(ZInputPin);
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("VECTOR3");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(XInputPin);
            const float label_width = ImGui::CalcTextSize("x").x;
            ImGui::TextUnformatted("x");
            if (ParentGraph->GetNumEdgesFromNode(XInputPin) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel0", &XInputNodeValue->Value, 0.01f);
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
                ImGui::DragFloat("##hidelabel1", &YInputNodeValue->Value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginInputAttribute(ZInputPin);
            const float label_width = ImGui::CalcTextSize("z").x;
            ImGui::TextUnformatted("z");
            if (ParentGraph->GetNumEdgesFromNode(ZInputPin) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel2", &ZInputNodeValue->Value, 0.01f);
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
            << " " << ZInputPin
            << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Vector3;
        in >> Id
            >> XInputPin
            >> YInputPin
            >> ZInputPin
            >> OutputPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector3Node& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, Vector3Node& n)
    {
        return n.Deserialize(in);
    }
};