#pragma once

#include "Rendering/D3DUtil.h"

#include <fstream>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

using NodeId = int;

enum class NodeType
{
    None,
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
    Texture,
    Transform
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

struct HlslType
{
    NodeType NodeType;
    std::string TypeName;
    UINT Num32BitValues;
};

class HlslNodeType
{
public:
    static HlslNodeType* Get()
    {
        if (!_Instance)
            _Instance = new HlslNodeType;
        return _Instance;
    }

    HlslType GetHlslType(const std::string& typeName);
    HlslType GetHlslType(NodeType nodeType);

    UINT GetNum32BitValues(std::string typeName);
    UINT GetNum32BitValues(NodeType nodeType);


private:
    static HlslNodeType* _Instance;
    
    std::vector<HlslType> _HlslTypes;
    std::unordered_map<NodeType, size_t> _HlslTypeNodeMap;
    std::unordered_map<std::string, size_t> _HlslTypeNameMap;

    HlslNodeType() 
    {
        _HlslTypes = {
            { NodeType::Int,      "int",       1 },
            { NodeType::Float,    "float",     1 },
            { NodeType::Float2,   "float2",    2 },
            { NodeType::Float3,   "float3",    3 },
            { NodeType::Float4,   "float4",    4 },
            { NodeType::Float4x4, "float4x4", 16 }
        };

        // index map to search by NodeType
        for (size_t i = 0ull; i < _HlslTypes.size(); ++i)
            _HlslTypeNodeMap[ _HlslTypes[i].NodeType ] = i;

        // index map to search by TypeName
        for (size_t i = 0ull; i < _HlslTypes.size(); ++i)
            _HlslTypeNameMap[ _HlslTypes[i].TypeName ] = i;
    }
};

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

struct NodeValue
{
    NodeValue(NodeType t) : Type(t) {}

    virtual void SetValuePtr(void*) = 0;
    virtual void* GetValuePtr() = 0;

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        out << (int)Type;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        return in; // type is loaded before calling this method
    }

    friend std::ostream& operator<<(std::ostream& out, const NodeValue& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, NodeValue& n)
    {
        return n.Deserialize(in);
    }

protected:
    NodeType Type;
};

struct NodeValueInt : public NodeValue
{
    int Value;

public:
    NodeValueInt(int initValue = 0)
        : NodeValue(NodeType::Int), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(int*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        NodeValue::Serialize(out);
        out << " " << Value;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        NodeValue::Deserialize(in);
        in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const NodeValueInt& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, NodeValueInt& n) { return n.Deserialize(in); }
};

struct NodeValueFloat : public NodeValue
{
    float Value;

public:
    NodeValueFloat(float initValue = 0.f)
        : NodeValue(NodeType::Float), Value(initValue)
    {}

    virtual void SetValuePtr(void* newValue) override { Value = *(float*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        NodeValue::Serialize(out);
        out << " " << Value;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        NodeValue::Deserialize(in);
        in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const NodeValueFloat& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, NodeValueFloat& n) { return n.Deserialize(in); }
};

struct NodeValueFloat4x4 : public NodeValue
{
    DirectX::XMFLOAT4X4 Value;

public:
    NodeValueFloat4x4(DirectX::XMFLOAT4X4 initValue = D3DUtil::Identity4x4())
        : NodeValue(NodeType::Float4x4), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT4X4*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        NodeValue::Serialize(out);
        out << " " << Value._11 << " " << Value._12 << " " << Value._13 << " " << Value._14;
        out << " " << Value._21 << " " << Value._22 << " " << Value._23 << " " << Value._24;
        out << " " << Value._31 << " " << Value._32 << " " << Value._33 << " " << Value._34;
        out << " " << Value._41 << " " << Value._42 << " " << Value._43 << " " << Value._44;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        NodeValue::Deserialize(in);
        in >> Value._11 >> Value._12 >> Value._13 >> Value._14;
        in >> Value._21 >> Value._22 >> Value._23 >> Value._24;
        in >> Value._31 >> Value._32 >> Value._33 >> Value._34;
        in >> Value._41 >> Value._42 >> Value._43 >> Value._44;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const NodeValueFloat4x4& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, NodeValueFloat4x4& n) { return n.Deserialize(in); }
};

struct NodeValueFloat4 : public NodeValue
{
    DirectX::XMFLOAT4 Value;

public:
    NodeValueFloat4(DirectX::XMFLOAT4 initValue = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f))
        : NodeValue(NodeType::Float4), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT4*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        NodeValue::Serialize(out);
        out << " " << Value.x << " " << Value.y << " " << Value.z << " " << Value.w;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        NodeValue::Deserialize(in);
        in >> Value.x >> Value.y >> Value.z >> Value.w;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const NodeValueFloat4& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, NodeValueFloat4& n) { return n.Deserialize(in); }
};

struct NodeValueFloat3 : public NodeValue
{
    DirectX::XMFLOAT3 Value;

public:
    NodeValueFloat3(DirectX::XMFLOAT3 initValue = DirectX::XMFLOAT3(0.f, 0.f, 0.f))
        : NodeValue(NodeType::Float3), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT3*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        NodeValue::Serialize(out);
        out << " " << Value.x << " " << Value.y << " " << Value.z;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        NodeValue::Deserialize(in);
        in >> Value.x >> Value.y >> Value.z;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const NodeValueFloat3& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, NodeValueFloat3& n) { return n.Deserialize(in); }
};

struct NodeValueFloat2 : public NodeValue
{
    DirectX::XMFLOAT2 Value;

public:
    NodeValueFloat2(DirectX::XMFLOAT2 initValue = DirectX::XMFLOAT2(0.f, 0.f))
        : NodeValue(NodeType::Float2), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT2*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        NodeValue::Serialize(out);
        out << " " << Value.x << " " << Value.y;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        NodeValue::Deserialize(in);
        in >> Value.x >> Value.y;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const NodeValueFloat2& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, NodeValueFloat2& n) { return n.Deserialize(in); }
};
