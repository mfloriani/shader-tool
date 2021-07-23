#pragma once

#include "Editor\Graph\Graph.h"
#include "Editor\Graph\Node.h"

#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"
#include "Editor/nfd/nfd.h"

using UiNodeId = int;

const static int INVALID_ID = -1;

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
    UiNode() : Type(UiNodeType::None), Id(INVALID_ID) {}
    explicit UiNode(UiNodeType type, UiNodeId id) : Type(type), Id(id) {}
    
    UiNodeType Type;
    UiNodeId Id;

    virtual void Render(Graph<Node>& graph) {}
    virtual void Delete(Graph<Node>& graph) {}

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