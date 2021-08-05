#pragma once

#include "UiNode.h"

struct MultiplyNode : UiNode
{
private:

public:
    NodeId LeftPin, RightPin, OutputPin;

    std::shared_ptr<NodeValue<float>> LeftNodeValue;
    std::shared_ptr<NodeValue<float>> RightNodeValue;
    std::shared_ptr<NodeValue<float>> OutputNodeValue;

public:
    explicit MultiplyNode(Graph* graph)
        : UiNode(graph, UiNodeType::Multiply), LeftPin(INVALID_ID), RightPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        LeftNodeValue = std::make_shared<NodeValue<float>>();
        LeftNodeValue->TypeName = "float";
        LeftNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[LeftNodeValue->TypeName];
        LeftNodeValue->Data = 0.f;

        RightNodeValue = std::make_shared<NodeValue<float>>();
        RightNodeValue->TypeName = "float";
        RightNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[RightNodeValue->TypeName];
        RightNodeValue->Data = 0.f;

        OutputNodeValue = std::make_shared<NodeValue<float>>();
        OutputNodeValue->TypeName = "float";
        OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = 0.f;
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Multiply, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node floatNodeIn(NodeType::Float, NodeDirection::In);
        LeftPin = ParentGraph->CreateNode(floatNodeIn);
        RightPin = ParentGraph->CreateNode(floatNodeIn);

        const Node floatNodeOut(NodeType::Float, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(floatNodeOut);

        ParentGraph->CreateEdge(Id, LeftPin, EdgeType::Internal);
        ParentGraph->CreateEdge(Id, RightPin, EdgeType::Internal);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);

        StoreNodeValuePtr<float>(LeftPin, LeftNodeValue);
        StoreNodeValuePtr<float>(RightPin, RightNodeValue);
        StoreNodeValuePtr<float>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<float>(LeftPin, LeftNodeValue);
        StoreNodeValuePtr<float>(RightPin, RightNodeValue);
        StoreNodeValuePtr<float>(OutputPin, OutputNodeValue);
    }

    virtual void OnEval() override
    {
        OutputNodeValue->Data = LeftNodeValue->Data * RightNodeValue->Data;
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(LeftPin);
        ParentGraph->EraseNode(RightPin);
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("MULTIPLY");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(LeftPin);
            const float label_width = ImGui::CalcTextSize("left").x;
            ImGui::TextUnformatted("left");
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginInputAttribute(RightPin);
            const float label_width = ImGui::CalcTextSize("right").x;
            ImGui::TextUnformatted("right");
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginOutputAttribute(OutputPin);
            const float label_width = ImGui::CalcTextSize("output").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("output");
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << LeftPin << " " << RightPin << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Multiply;
        in >> Id >> LeftPin >> RightPin >> OutputPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const MultiplyNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, MultiplyNode& n)
    {
        return n.Deserialize(in);
    }
};