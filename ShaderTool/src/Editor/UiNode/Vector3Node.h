#pragma once

#include "UiNode.h"

struct Vector3Node : UiNode
{
private:

public:
    NodeId XInputPin, YInputPin, ZInputPin, OutputPin;
    std::shared_ptr<NodeValue<float>> XInputNodeValue;
    std::shared_ptr<NodeValue<float>> YInputNodeValue;
    std::shared_ptr<NodeValue<float>> ZInputNodeValue;
    std::shared_ptr<NodeValue<DirectX::XMFLOAT3>> OutputNodeValue;

public:
    explicit Vector3Node(Graph* graph)
        : UiNode(graph, UiNodeType::Vector3),
        XInputPin(INVALID_ID), YInputPin(INVALID_ID), ZInputPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        XInputNodeValue = std::make_shared<NodeValue<float>>();
        XInputNodeValue->TypeName = "float";
        XInputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[XInputNodeValue->TypeName];
        XInputNodeValue->Data = 0.f;

        YInputNodeValue = std::make_shared<NodeValue<float>>();
        YInputNodeValue->TypeName = "float";
        YInputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[YInputNodeValue->TypeName];
        YInputNodeValue->Data = 0.f;

        ZInputNodeValue = std::make_shared<NodeValue<float>>();
        ZInputNodeValue->TypeName = "float";
        ZInputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[ZInputNodeValue->TypeName];
        ZInputNodeValue->Data = 0.f;

        OutputNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT3>>();
        OutputNodeValue->TypeName = "float3";
        OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Vector3, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node inputNode(NodeType::Float, NodeDirection::In);

        XInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, XInputPin, EdgeType::Internal);
        StoreNodeValuePtr<float>(XInputPin, XInputNodeValue);

        YInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, YInputPin, EdgeType::Internal);
        StoreNodeValuePtr<float>(YInputPin, YInputNodeValue);

        ZInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, ZInputPin, EdgeType::Internal);
        StoreNodeValuePtr<float>(ZInputPin, ZInputNodeValue);

        const Node outputNode(NodeType::Float3, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        StoreNodeValuePtr<DirectX::XMFLOAT3>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<float>(XInputPin, XInputNodeValue);
        StoreNodeValuePtr<float>(YInputPin, YInputNodeValue);
        StoreNodeValuePtr<float>(ZInputPin, ZInputNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT3>(OutputPin, OutputNodeValue);
    }


    virtual void OnEval() override
    {
        OutputNodeValue->Data = DirectX::XMFLOAT3(
            XInputNodeValue->Data,
            YInputNodeValue->Data,
            ZInputNodeValue->Data);
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
                ImGui::DragFloat("##hidelabel", &XInputNodeValue->Data, 0.01f);
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
                ImGui::DragFloat("##hidelabel", &YInputNodeValue->Data, 0.01f);
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
                ImGui::DragFloat("##hidelabel", &ZInputNodeValue->Data, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginOutputAttribute(OutputPin);
            const float label_width = ImGui::CalcTextSize("output").x;
            ImGui::Indent(node_width - label_width);
            ImGui::Text("output");
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
            << " " << OutputPin
            << " " << XInputNodeValue->Data
            << " " << YInputNodeValue->Data
            << " " << ZInputNodeValue->Data;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Vector3;
        in >> Id
            >> XInputPin
            >> YInputPin
            >> ZInputPin
            >> OutputPin
            >> XInputNodeValue->Data
            >> YInputNodeValue->Data
            >> ZInputNodeValue->Data;
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