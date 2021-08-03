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
    None,
    Add,
    Multiply,
    Draw,
    Sine,
    Time,
    RenderTarget,
    Primitive,
    Shader,
};

struct UiNode : public IObserver
{
    explicit UiNode(Graph<Node>* graph, UiNodeType type) : ParentGraph(graph), Type(type), Id(INVALID_ID) {}
    
    virtual ~UiNode()
    {
        LOG_TRACE("~UiNode()");
        if(ParentGraph) ParentGraph->EraseNode(Id);
    }

    Graph<Node>* ParentGraph;
    UiNodeType Type;
    NodeId Id;

    virtual void OnEvent(Event*) = 0;
    virtual void OnCreate() = 0;
    virtual void OnDelete() = 0;
    virtual void OnUpdate(GameTimer& timer) = 0;
    virtual void OnRender() = 0;

    void SetPinValue(NodeId id, float value) { ParentGraph->GetNode(id).Value = value; }
    float GetPinValue(NodeId id) const { return ParentGraph->GetNode(id).Value; }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        out << static_cast<int>(Type) << " " << Id;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        int type;
        in >> type >> Id;
        Type = static_cast<UiNodeType>(type);
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