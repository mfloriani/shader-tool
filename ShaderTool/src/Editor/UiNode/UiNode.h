#pragma once

#include "GameTimer.h"
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
    Scalar
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