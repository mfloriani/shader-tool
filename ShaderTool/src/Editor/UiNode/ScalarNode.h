#pragma once

#include "UiNode.h"

struct ScalarNode : UiNode
{
private:
    
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

    virtual void OnUpdate() override
    {

    }

    virtual void OnEval() override
    {
        
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
        ImGui::DragFloat("##hidelabel", &OutputNodeValue->Data, 0.01f);
        ImGui::PopItemWidth();

        ImNodes::BeginOutputAttribute(OutputPin);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin << " " << OutputNodeValue->Data;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Scalar;
        in >> Id >> OutputPin >> OutputNodeValue->Data;
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