#pragma once

#include "UiNode.h"
#include "AssetManager.h"

struct TextureNode : UiNode
{
private:
    Texture* _Texture;
    std::string _Path;

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValueInt> OutputNodeValue;

public:
    explicit TextureNode(Graph* graph)
        : UiNode(graph, UiNodeType::Texture), _Texture(nullptr)
    {
        OutputNodeValue = std::make_shared<NodeValueInt>(INVALID_INDEX);
    }

    const std::string GetPath() const 
    { 
        if (OutputNodeValue->Value != INVALID_INDEX)
            return AssetManager::Get().GetTexture(OutputNodeValue->Value)->Path;

        return "";
    }

    const std::string GetName() const 
    { 
        if (OutputNodeValue->Value != INVALID_INDEX)
            return AssetManager::Get().GetTexture(OutputNodeValue->Value)->Name;

        return "";
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        OutputNodeValue->Value = INVALID_INDEX;

        const Node idNode = Node(NodeType::Texture, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node indexNodeOut(NodeType::Int, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(indexNodeOut);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        OutputNodeValue->Value = *(int*)ParentGraph->GetNodeValue(OutputPin)->GetValuePtr();
        if (OutputNodeValue->Value != INVALID_INDEX)
        {
            OutputNodeValue->Value = AssetManager::Get().LoadTextureFromFile(_Path);
            _Texture = AssetManager::Get().GetTexture(OutputNodeValue->Value);
            //_Texture = AssetManager::Get().LoadTextureFromIndex(OutputNodeValue->Value);
            if (!_Texture || _Texture->Name.empty())
            {
                LOG_ERROR("Failed to load texture from index {0}!", OutputNodeValue->Value);
                OutputNodeValue->Value = INVALID_INDEX;
                _Texture = nullptr;
            }
        }
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
        ImGui::TextUnformatted("TEXTURE");
        ImNodes::EndNodeTitleBar();

        if (ImGui::Button("Open"))
        {
            nfdchar_t* outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("dds", NULL, &outPath);

            if (result == NFD_OKAY)
            {
                _Path = std::string(outPath);
                OutputNodeValue->Value = AssetManager::Get().LoadTextureFromFile(_Path);
                _Texture = AssetManager::Get().GetTexture(OutputNodeValue->Value);
                ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
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
        
        // TODO: temp dimensions
        static int w = 256;
        static int h = 256;
        ImNodes::BeginOutputAttribute(OutputPin);
        if(_Texture) 
            ImGui::Image((ImTextureID)_Texture->SrvGpuDescHandle.ptr, ImVec2((float)w, (float)h));
        ImNodes::EndOutputAttribute();

        if (OutputNodeValue->Value != INVALID_INDEX)
            ImGui::Text(GetName().c_str());

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin << " " << _Path;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Texture;
        in >> Id >> OutputPin >> _Path;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const TextureNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, TextureNode& n)
    {
        return n.Deserialize(in);
    }
};
