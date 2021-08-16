#pragma once

#include "UiNode.h"

struct PrimitiveNode : UiNode
{
private:
    std::vector<std::string> _PrimitiveNames;               // list of all primitives
    std::unordered_map<size_t, int> _PrimitiveNameIndexMap; // maps internal vector index with model index
    int _SelectedModel;                                     // stores the internal index of primitives (do not return this)

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValueInt> OutputNodeValue;

public:
    explicit PrimitiveNode(Graph* graph, std::vector<int>& primitives)
        : UiNode(graph, UiNodeType::Primitive), _SelectedModel(0), OutputPin(INVALID_ID)
    {
        OutputNodeValue = std::make_shared<NodeValueInt>(INVALID_INDEX);

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
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {
        
    }
    
    virtual void OnEval() override
    {
        OutputNodeValue->Value = _PrimitiveNameIndexMap[_SelectedModel];
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
        ImGui::Combo("##hidelabel", &_SelectedModel, items.data(), (int)items.size());
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