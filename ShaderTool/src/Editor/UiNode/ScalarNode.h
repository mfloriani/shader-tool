#pragma once

#include "UiNode.h"

struct ScalarNode : UiNode
{
private:
    float _Value{ 0 };

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValue<float>> OutputNodeValue;

public:
    explicit ScalarNode(Graph* graph)
        : UiNode(graph, UiNodeType::Scalar), OutputPin(INVALID_ID)
    {
        OutputNodeValue = std::make_shared<NodeValue<float>>();
        OutputNodeValue->TypeName = "float";
        OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = 0.f;
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Scalar, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node outputNode(NodeType::Float, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        StoreNodeValuePtr<float>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<float>(OutputPin, OutputNodeValue);
    }


    virtual void OnEval() override
    {
        OutputNodeValue->Data = _Value;
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
        ImGui::TextUnformatted("SCALAR");
        ImNodes::EndNodeTitleBar();

        ImGui::PushItemWidth(node_width);
        ImGui::InputFloat("", &_Value);
        ImGui::PopItemWidth();

        ImNodes::BeginOutputAttribute(OutputPin);
        const float label_width = ImGui::CalcTextSize("output (float)").x;
        ImGui::Indent(node_width - label_width);
        ImGui::Text("output (float)");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin << " " << _Value;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Scalar;
        in >> Id >> OutputPin >> _Value;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const ScalarNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, ScalarNode& n)
    {
        return n.Deserialize(in);
    }
};