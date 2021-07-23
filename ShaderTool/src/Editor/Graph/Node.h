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
    NodeType    Type;
    float       Value;
    std::string TypeName;

    explicit Node(const NodeType t) : Type(t), Value(0.f) { TypeName = magic_enum::enum_name(t); }
    Node(const NodeType t, const float v) : Type(t), Value(v) { TypeName = magic_enum::enum_name(t); }

    friend std::ostream& operator<<(std::ostream& out, const Node& n)
    {
        out << static_cast<int>(n.Type)
            << " "
            << n.Value;
        return out;
    }

    friend std::istream& operator>>(std::istream& in, Node& n)
    {
        int type;
        in >> type >> n.Value;
        n.Type = static_cast<NodeType>(type);
        return in;
    }
};
