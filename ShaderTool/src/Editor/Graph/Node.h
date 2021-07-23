#pragma once

#include <fstream>

enum class NodeType
{
    Value,
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

using NodeId = int;

struct Node
{
    NodeType type;
    float    value;

    explicit Node(const NodeType t) : type(t), value(0.f) {}
    Node(const NodeType t, const float v) : type(t), value(v) {}

    friend std::ostream& operator<<(std::ostream& out, const Node& n)
    {
        out << static_cast<int>(n.type)
            << " "
            << n.value;
        return out;
    }

    friend std::istream& operator>>(std::istream& in, Node& n)
    {
        int type;
        in >> type >> n.value;
        n.type = static_cast<NodeType>(type);
        return in;
    }
};
