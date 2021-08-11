#pragma once

#include "UiNode.h"
#include "AssetManager.h"

struct ModelNode : UiNode
{
private:
    std::string _Path;
    int _ModelIndex;

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValue<int>> OutputNodeValue;

public:
    explicit ModelNode(Graph* graph)
        : UiNode(graph, UiNodeType::Model), _Path(""), _ModelIndex(INVALID_INDEX)
    {
        OutputNodeValue = std::make_shared<NodeValue<int>>();
        OutputNodeValue->TypeName = "int";
        //OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
        OutputNodeValue->Data = INVALID_INDEX;
    }

    const std::string GetPath() const { return _Path; }
    const std::string GetName() const { return _ModelIndex == INVALID_INDEX ? "" : AssetManager::Get().GetModel(_ModelIndex).Name; }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        OutputNodeValue->Data = INVALID_INDEX;

        const Node idNode = Node(NodeType::Model, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node indexNodeOut(NodeType::Int, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(indexNodeOut);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);

        StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        Model model = AssetManager::Get().GetModel(_ModelIndex);
        if(model.Name.empty())
        {
            _ModelIndex = AssetManager::Get().LoadModelFromFile(_Path);
            if (_ModelIndex == INVALID_INDEX)
            {
                LOG_ERROR("Failed to load model {0}!", _Path);
                _Path = "";
                OutputNodeValue->Data = INVALID_INDEX;
                StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
                return;
            }
        }
        else
        {
            OutputNodeValue->Data = _ModelIndex;
            StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
        }
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
        ImGui::TextUnformatted("MODEL");
        ImNodes::EndNodeTitleBar();

        if (ImGui::Button("Open"))
        {
            nfdchar_t* outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("obj", NULL, &outPath);

            if (result == NFD_OKAY)
            {
                _Path = std::string(outPath);
                _ModelIndex = AssetManager::Get().LoadModelFromFile(_Path);
                OnLoad();
                free(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                LOG_TRACE("User pressed cancel.");
            }
            else
            {
                LOG_ERROR("Error: %s\n", NFD_GetError());
            }
        }
        if(_ModelIndex != INVALID_INDEX)
            ImGui::Text(GetName().c_str());

        ImNodes::BeginOutputAttribute(OutputPin);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin << " " << _ModelIndex << " " << _Path;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Model;
        in >> Id >> OutputPin >> _ModelIndex >> _Path;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const ModelNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, ModelNode& n)
    {
        return n.Deserialize(in);
    }
};
