#pragma once

#include "UiNode.h"

struct PrimitiveNode : UiNode
{
private:
    std::vector<const char*>& _Primitives;
    int _SelectedModel;

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValue<int>> OutputNodeValue;

public:
    explicit PrimitiveNode(Graph* graph, std::vector<const char*>& primitives)
        : UiNode(graph, UiNodeType::Primitive), _Primitives(primitives), _SelectedModel(0), OutputPin(INVALID_ID)
    {
        OutputNodeValue = std::make_shared<NodeValue<int>>();
        OutputNodeValue->TypeName = "int";
        OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = INVALID_INDEX;
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Primitive, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node outputNode(NodeType::Int, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        
        StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {

    }
    
    virtual void OnEval() override
    {
        GetNodeValuePtr<int>(OutputPin)->Data = _SelectedModel;
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
        ImGui::TextUnformatted("PRIMITIVE");
        ImNodes::EndNodeTitleBar();

        ImGui::PushItemWidth(node_width);
        ImGui::Combo("##hidelabel", &_SelectedModel, _Primitives.data(), (int)_Primitives.size());
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
        out << " " << OutputPin << " " << _SelectedModel;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Primitive;
        in >> Id >> OutputPin >> _SelectedModel;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const PrimitiveNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, PrimitiveNode& n)
    {
        return n.Deserialize(in);
    }
};