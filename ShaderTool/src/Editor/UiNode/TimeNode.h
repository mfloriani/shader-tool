#pragma once

#include "UiNode.h"

struct TimeNode : UiNode
{
private:
    GameTimer _Timer;

public:
    NodeId OutputPin;
    std::shared_ptr<GraphNodeValueFloat> OutputNodeValue;

public:
    explicit TimeNode(Graph* graph)
        : UiNode(graph, UiNodeType::Time), OutputPin(INVALID_ID)
    {
        OutputNodeValue = std::make_shared<GraphNodeValueFloat>(0.f);
        _Timer.Start();
    }

    virtual void OnEvent(Event* e) override {}
        
    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Time, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node outputNode(NodeType::Float, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(OutputPin);
    }
    
    virtual void OnUpdate() override
    {
        _Timer.Tick();
    }

    virtual void OnEval() override
    {
        OutputNodeValue->Value = _Timer.TotalTime();
    }

    virtual void OnRender() override
    {
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("TIME");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginOutputAttribute(OutputPin);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Time;
        in >> Id >> OutputPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const TimeNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, TimeNode& n)
    {
        return n.Deserialize(in);
    }
};
