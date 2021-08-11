#pragma once

#include "GameTimer.h"
#include "Defines.h"
#include "Patterns/IObserver.h"

#include "Editor\Graph\Graph.h"
#include "Editor\Graph\Node.h"
#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"
#include "Editor/nfd/nfd.h"

#include "Rendering/D3DUtil.h"

enum class UiNodeType
{
    Add,
    Multiply,
    Draw,
    Sine,
    Time,
    RenderTarget,
    Primitive,
    Shader,
    Color,
    Camera,
    Scalar,
    Vector4,
    Vector3,
    Vector2,
    Matrix4x4,
    Model,
    Texture,
    Transform
};

struct UiNode : public IObserver
{
protected:
    Graph* ParentGraph;

public:
    explicit UiNode(Graph* graph, UiNodeType type) : ParentGraph(graph), Type(type), Id(INVALID_ID) {}
    
    virtual ~UiNode()
    {
        LOG_TRACE("~UiNode()");
        if(ParentGraph) ParentGraph->EraseNode(Id);
    }

    
    UiNodeType Type;
    NodeId Id;

    virtual void OnEvent(Event*) = 0;
    virtual void OnCreate() = 0;
    virtual void OnLoad() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnDelete() = 0;
    virtual void OnRender() = 0;
    virtual void OnEval() = 0;
    
    template <typename T>
    void StoreNodeValuePtr(NodeId id, std::shared_ptr<NodeValue<T>> value)
    {
        GraphNodeValues<T>::Get().StoreNodeValuePtr(id, value);
    }

    template <typename T>
    std::shared_ptr<NodeValue<T>>& GetNodeValuePtr(NodeId id)
    {
        return GraphNodeValues<T>::Get().GetNodeValuePtr(id);
    }

    void StoreNodeValue(NodeId id, std::shared_ptr<GraphNodeValue> value)
    {
        ParentGraph->StoreNodeValue(id, value);
    }

    bool CopyValueFromLinkedSource(NodeId id, void* defaultValue)
    {
        size_t numNeighbors = ParentGraph->GetNumEdgesFromNode(id);
        if (numNeighbors == 1ull) // there is one link
        {
            NodeId neighborId = *ParentGraph->GetNeighbors(id).begin();
            auto neighborValue = ParentGraph->GetNodeValue(neighborId)->GetValue();
            ParentGraph->GetNodeValue(id)->SetValue( neighborValue );
            return true;
        }
        
        if (numNeighbors > 1ull)
            LOG_ERROR("Multiple links [{0}] at node {1}", numNeighbors, id);

        ParentGraph->GetNodeValue(id)->SetValue(defaultValue);

        return false;
    }

    template <typename T>
    bool CopyFromLinkedSourceNodeValue(NodeId id, T defaultValue)
    {
        size_t numNeighbors = ParentGraph->GetNumEdgesFromNode(id);
        if (numNeighbors == 1ull) // there is one link
        {
            NodeId neighborId = *ParentGraph->GetNeighbors(id).begin();
            GetNodeValuePtr<T>(id)->Data = GetNodeValuePtr<T>(neighborId)->Data;
            return true;
        }
        
        // no link or more than one

        if (numNeighbors > 1ull)
            LOG_ERROR("Multiple links [{0}] at node {1}", numNeighbors, id);

        GetNodeValuePtr<T>(id)->Data = defaultValue;

        return false;
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        out << magic_enum::enum_name(Type) << " " << static_cast<int>(Type) << " " << Id;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const UiNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, UiNode& n)
    {
        return n.Deserialize(in);
    }
};