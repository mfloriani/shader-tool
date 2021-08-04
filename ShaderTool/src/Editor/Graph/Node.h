#pragma once

#include <fstream>
#include <DirectXMath.h>

const static int INVALID_ID = -1;
const static int NOT_LINKED = -1;
const static int INVALID_INDEX = -1;

enum class NodeType
{
    Value,
    Link,
    Add,
    Multiply,
    Draw,
    Sine,
    Time,
    RenderTarget,
    Primitive,
    Shader,
    Color
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


struct NodeValue
{
};

struct NodeFloat : public NodeValue
{
    NodeFloat(float v) : value(v) {}
    float value;
};

struct NodeInt : public NodeValue
{
    NodeInt(int v) : value(v) {}
    int value;
};

struct NodeFloat3 : public NodeValue
{
    NodeFloat3(float x) : value(x, x, x) {}
    NodeFloat3(float x, float y, float z) : value(x, y, z) {}
    DirectX::XMFLOAT3 value;
};

struct NodeFloat4 : public NodeValue
{
    NodeFloat4(float x) : value(x, x, x, x) {}
    NodeFloat4(float x, float y, float z, float w) : value(x, y, z, w) {}
    DirectX::XMFLOAT4 value;
};

struct NodeFloat4X4 : public NodeValue
{
    //NodeFloat4X4() : value() {}
    DirectX::XMFLOAT4X4 value;
};
