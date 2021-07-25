#pragma once

#include "Editor\Graph\Graph.h"
#include "Editor\Graph\Node.h"

#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"
#include "Editor/nfd/nfd.h"

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
    VertexShader,
    PixelShader
};

struct UiNode
{
    explicit UiNode(Graph<Node>* graph, UiNodeType type) : ParentGraph(graph), Type(type), Id(INVALID_ID) {}
    
    virtual ~UiNode()
    {
        LOG_TRACE("~UiNode()");
        if(ParentGraph)
            ParentGraph->EraseNode(Id);
    }

    Graph<Node>* ParentGraph;
    UiNodeType Type;
    NodeId Id;

    virtual void OnCreate() = 0;
    virtual void OnDelete() = 0;
    virtual void OnUpdate(GameTimer& timer) = 0;
    virtual void OnRender() = 0;

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