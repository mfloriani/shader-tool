#pragma once

#include "UiNode.h"

struct PrimitiveNode : UiNode
{
private:
    std::vector<std::string> _PrimitiveNames;               // list of all primitives
    std::unordered_map<size_t, int> _PrimitiveNameIndexMap; // maps internal vector index with model index

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValueInt> OutputNodeValue;

public:
    explicit PrimitiveNode(Graph* graph, std::vector<int>& primitives)
        : UiNode(graph, UiNodeType::Primitive), OutputPin(INVALID_ID)
    {
        OutputNodeValue = std::make_shared<NodeValueInt>(0);

        for (int p : primitives)
        {
            auto model = AssetManager::Get().GetModel(p);
            _PrimitiveNames.push_back(model.Name);
            _PrimitiveNameIndexMap[_PrimitiveNames.size()-1] = p;
        }
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Primitive, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node outputNode(NodeType::Int, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(outputNode);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        OutputNodeValue->Value = *(int*)ParentGraph->GetNodeValue(OutputPin)->GetValuePtr();
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
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
        ImGui::TextUnformatted("PRIMITIVE");
        ImNodes::EndNodeTitleBar();

        std::vector<const char*> items;
        for (int i = 0; i < _PrimitiveNames.size(); ++i)
            items.push_back(_PrimitiveNames[i].c_str());

        ImGui::PushItemWidth(node_width);
        ImGui::Combo("##hidelabel", &OutputNodeValue->Value, items.data(), (int)items.size());
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
        out << " " << OutputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Primitive;
        in >> Id >> OutputPin;
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