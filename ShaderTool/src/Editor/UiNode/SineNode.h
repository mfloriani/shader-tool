#pragma once

#include "UiNode.h"

struct SineNode : UiNode
{
private:

public:
    NodeId InputPin, OutputPin;
    std::shared_ptr<NodeValue<float>> InputNodeValue;
    std::shared_ptr<NodeValue<float>> OutputNodeValue;

public:
    explicit SineNode(Graph* graph)
        : UiNode(graph, UiNodeType::Sine), InputPin(INVALID_ID), OutputPin(INVALID_ID)
    {
        InputNodeValue = std::make_shared<NodeValue<float>>();
        InputNodeValue->TypeName = "float";
        //InputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[InputNodeValue->TypeName];
        InputNodeValue->Data = 0.f;

        OutputNodeValue = std::make_shared<NodeValue<float>>();
        OutputNodeValue->TypeName = "float";
        //OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = 0.f;
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Sine, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node inputNode(NodeType::Float, NodeDirection::In);
        InputPin = ParentGraph->CreateNode(inputNode);

        const Node outputNode(NodeType::Float, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);

        ParentGraph->CreateEdge(Id, InputPin, EdgeType::Internal);
        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);

        StoreNodeValuePtr<float>(InputPin, InputNodeValue);
        StoreNodeValuePtr<float>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<float>(InputPin, InputNodeValue);
        StoreNodeValuePtr<float>(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {

    }
    
    virtual void OnEval() override
    {
        CopyFromLinkedSourceNodeValue<float>(InputPin, 0.f);
        OutputNodeValue->Data = std::abs(std::sin( InputNodeValue->Data ));
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(InputPin);
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("SINE");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(InputPin);
            const float label_width = ImGui::CalcTextSize("input").x;
            ImGui::TextUnformatted("input");
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginOutputAttribute(OutputPin);
            const float label_width = ImGui::CalcTextSize("output").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("output");
            ImNodes::EndInputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << InputPin << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Sine;
        in >> Id >> InputPin >> OutputPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const SineNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, SineNode& n)
    {
        return n.Deserialize(in);
    }
};