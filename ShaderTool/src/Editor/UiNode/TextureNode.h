#pragma once

#include "UiNode.h"
#include "AssetManager.h"

struct TextureNode : UiNode
{
private:
    std::string _Path;
    int _TextureIndex;
    Texture* _Texture;

public:
    NodeId OutputPin;
    std::shared_ptr<GraphNodeValueInt> OutputNodeValue;

public:
    explicit TextureNode(Graph* graph)
        : UiNode(graph, UiNodeType::Texture), _Path(""), _TextureIndex(INVALID_INDEX), _Texture(nullptr)
    {
        OutputNodeValue = std::make_shared<GraphNodeValueInt>(INVALID_INDEX);
    }

    const std::string GetPath() const { return _Path; }
    const std::string GetName() const { return _TextureIndex == INVALID_INDEX ? "" : AssetManager::Get().GetTexture(_TextureIndex)->Name; }

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
        _Texture = AssetManager::Get().GetTexture(_TextureIndex);
        if (_Texture->Name.empty())
        {
            _TextureIndex = AssetManager::Get().LoadTextureFromFile(_Path);
            if (_TextureIndex == INVALID_INDEX)
            {
                LOG_ERROR("Failed to load texture {0}!", _Path);
                _Path = "";
                _Texture = nullptr;
                OutputNodeValue->Value = INVALID_INDEX;
                ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
                return;
            }
        }
        else
        {
            OutputNodeValue->Value = _TextureIndex;
            ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
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
        ImGui::TextUnformatted("TEXTURE");
        ImNodes::EndNodeTitleBar();

        if (ImGui::Button("Open"))
        {
            nfdchar_t* outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("dds", NULL, &outPath);

            if (result == NFD_OKAY)
            {
                _Path = std::string(outPath);
                _TextureIndex = AssetManager::Get().LoadTextureFromFile(_Path);
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
        
        ImNodes::BeginOutputAttribute(OutputPin);
        // TODO: temp dimensions
        static int w = 256;
        static int h = 256;
        if(_Texture) 
            ImGui::Image((ImTextureID)_Texture->SrvGpuDescHandle.ptr, ImVec2((float)w, (float)h));
        ImNodes::EndOutputAttribute();

        if (_TextureIndex != INVALID_INDEX)
            ImGui::Text(GetName().c_str());

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin << " " << _TextureIndex << " " << _Path;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Texture;
        in >> Id >> OutputPin >> _TextureIndex >> _Path;
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
