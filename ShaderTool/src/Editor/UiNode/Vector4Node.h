#pragma once

#include "UiNode.h"

struct Vector4Node : UiNode
{
private:
    
public:
    NodeId XInputPin, YInputPin, ZInputPin, WInputPin, OutputPin;
    std::shared_ptr<NodeValue<float>> XInputNodeValue;
    std::shared_ptr<NodeValue<float>> YInputNodeValue;
    std::shared_ptr<NodeValue<float>> ZInputNodeValue;
    std::shared_ptr<NodeValue<float>> WInputNodeValue;
    std::shared_ptr<NodeValue<DirectX::XMFLOAT4>> OutputNodeValue;

public:
    explicit Vector4Node(Graph* graph)
        : UiNode(graph, UiNodeType::Vector4), 
        XInputPin(INVALID_ID), YInputPin(INVALID_ID), ZInputPin(INVALID_ID), WInputPin(INVALID_ID), OutputPin(INVALID_ID)
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

        WInputNodeValue = std::make_shared<NodeValue<float>>();
        WInputNodeValue->TypeName = "float";
        WInputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[WInputNodeValue->TypeName];
        WInputNodeValue->Data = 0.f;

        OutputNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT4>>();
        OutputNodeValue->TypeName = "float4";
        OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f);
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Vector4, NodeDirection::None);
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

        WInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, WInputPin, EdgeType::Internal);
        StoreNodeValuePtr<float>(WInputPin, WInputNodeValue);

        const Node outputNode(NodeType::Float4, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        StoreNodeValuePtr<DirectX::XMFLOAT4>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<float>(XInputPin, XInputNodeValue);
        StoreNodeValuePtr<float>(YInputPin, YInputNodeValue);
        StoreNodeValuePtr<float>(ZInputPin, ZInputNodeValue);
        StoreNodeValuePtr<float>(WInputPin, WInputNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT4>(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {

    }

    virtual void OnEval() override
    {
        CopyFromLinkedSourceNodeValue<float>(XInputPin, 0.f);
        CopyFromLinkedSourceNodeValue<float>(YInputPin, 0.f);
        CopyFromLinkedSourceNodeValue<float>(ZInputPin, 0.f);
        CopyFromLinkedSourceNodeValue<float>(WInputPin, 0.f);

        OutputNodeValue->Data = DirectX::XMFLOAT4(
            XInputNodeValue->Data,
            YInputNodeValue->Data,
            ZInputNodeValue->Data,
            WInputNodeValue->Data);
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(XInputPin);
        ParentGraph->EraseNode(YInputPin);
        ParentGraph->EraseNode(ZInputPin);
        ParentGraph->EraseNode(WInputPin);
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("VECTOR4");
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
            ImNodes::BeginInputAttribute(WInputPin);
            const float label_width = ImGui::CalcTextSize("w").x;
            ImGui::TextUnformatted("w");
            if (ParentGraph->GetNumEdgesFromNode(WInputPin) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &WInputNodeValue->Data, 0.01f);
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
            << " " << WInputPin 
            << " " << OutputPin
            << " " << XInputNodeValue->Data
            << " " << YInputNodeValue->Data
            << " " << ZInputNodeValue->Data
            << " " << WInputNodeValue->Data;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Vector4;
        in >> Id 
            >> XInputPin 
            >> YInputPin 
            >> ZInputPin 
            >> WInputPin 
            >> OutputPin
            >> XInputNodeValue->Data
            >> YInputNodeValue->Data
            >> ZInputNodeValue->Data
            >> WInputNodeValue->Data;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector4Node& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, Vector4Node& n)
    {
        return n.Deserialize(in);
    }
};