#pragma once

#include "UiNode.h"

struct Vector4Node : UiNode
{
private:
    
public:
    NodeId XInputPin, YInputPin, ZInputPin, WInputPin, OutputPin;
    
    std::shared_ptr<NodeValueFloat> XInputNodeValue;
    std::shared_ptr<NodeValueFloat> YInputNodeValue;
    std::shared_ptr<NodeValueFloat> ZInputNodeValue;
    std::shared_ptr<NodeValueFloat> WInputNodeValue;
    std::shared_ptr<NodeValueFloat4> OutputNodeValue;

public:
    explicit Vector4Node(Graph* graph)
        : UiNode(graph, UiNodeType::Vector4), 
        XInputPin(INVALID_ID), YInputPin(INVALID_ID), ZInputPin(INVALID_ID), WInputPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        XInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        YInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        ZInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        WInputNodeValue = std::make_shared<NodeValueFloat>(0.f);
        OutputNodeValue = std::make_shared<NodeValueFloat4>(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f));
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Vector4, NodeDirection::None);
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

        WInputPin = ParentGraph->CreateNode(inputNode);
        ParentGraph->CreateEdge(Id, WInputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(WInputPin, WInputNodeValue);

        const Node outputNode(NodeType::Float4, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        ParentGraph->StoreNodeValue(XInputPin, XInputNodeValue);
        ParentGraph->StoreNodeValue(YInputPin, YInputNodeValue);
        ParentGraph->StoreNodeValue(ZInputPin, ZInputNodeValue);
        ParentGraph->StoreNodeValue(WInputPin, WInputNodeValue);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {

    }

    virtual void OnEval() override
    {
        float defaultValue = 0.f;
        CopyValueFromLinkedSource(XInputPin, (void*)&defaultValue);
        CopyValueFromLinkedSource(YInputPin, (void*)&defaultValue);
        CopyValueFromLinkedSource(ZInputPin, (void*)&defaultValue);
        CopyValueFromLinkedSource(WInputPin, (void*)&defaultValue);

        OutputNodeValue->Value = DirectX::XMFLOAT4(
            XInputNodeValue->Value,
            YInputNodeValue->Value,
            ZInputNodeValue->Value,
            WInputNodeValue->Value);
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
            ImNodes::BeginInputAttribute(ZInputPin);
            const float label_width = ImGui::CalcTextSize("z").x;
            ImGui::TextUnformatted("z");
            if (ParentGraph->GetNumEdgesFromNode(ZInputPin) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &ZInputNodeValue->Value, 0.01f);
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
                ImGui::DragFloat("##hidelabel", &WInputNodeValue->Value, 0.01f);
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
            << " " << XInputNodeValue->Value
            << " " << YInputNodeValue->Value
            << " " << ZInputNodeValue->Value
            << " " << WInputNodeValue->Value;
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
            >> XInputNodeValue->Value
            >> YInputNodeValue->Value
            >> ZInputNodeValue->Value
            >> WInputNodeValue->Value;
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