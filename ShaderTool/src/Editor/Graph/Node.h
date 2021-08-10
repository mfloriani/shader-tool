#pragma once

#include <fstream>
#include <DirectXMath.h>



enum class NodeType
{
    Int,
    Float,
    Float2,
    Float3,
    Float4,
    Float4x4,
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
    Texture
};

// None for uinode ids
// In for nodes that receive data
// Out for nodes that produce data
enum class NodeDirection
{
    None,
    In,
    Out
};

using NodeId = int;

struct Node
{
    NodeType      Type;
    NodeDirection Direction;
    std::string   TypeName;
    std::string   DirectionName;

    Node() = default;
    explicit Node(const NodeType t, const NodeDirection d) 
        : Type(t), Direction(d) 
    { 
        TypeName = magic_enum::enum_name(t); 
        DirectionName = magic_enum::enum_name(d);
    }
    
    friend std::ostream& operator<<(std::ostream& out, const Node& n)
    {
        out << static_cast<int>(n.Type)
            << " "
            << n.TypeName
            << " "
            << static_cast<int>(n.Direction)
            << " "
            << magic_enum::enum_name(n.Direction);
        return out;
    }

    friend std::istream& operator>>(std::istream& in, Node& n)
    {
        int type, direction;
        in >> type >> n.TypeName >> direction >> n.DirectionName;
        n.Type = static_cast<NodeType>(type);
        n.Direction = static_cast<NodeDirection>(direction);
        return in;
    }
};

template<class T>
struct NodeValue
{
    std::string TypeName{""};
    UINT Num32BitValues{ 0 };
    T Data{};
};
