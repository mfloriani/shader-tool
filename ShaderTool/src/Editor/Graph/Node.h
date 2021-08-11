#pragma once

#include <DirectXMath.h>
#include <fstream>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

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
    Texture,
    Transform
};

//class NodeValueType
//{
//public:
//    struct HlslType
//    {
//        int Id;
//        std::string Name;
//        int Num32BitValues;
//    };
//
//public:
//    NodeValueType()
//    {
//
//    }
//
//    //Int,
//    //Float,
//    //Float2,
//    //Float3,
//    //Float4,
//    //Float4x4,
//
//private:
//    
//    std::unordered_map<std::string, int> _NodeValueTypeMap;
//};

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

//template<class T>
//struct NodeValue
//{
//    std::string TypeName{""};
//    //UINT Num32BitValues{ 0 };
//    T Data{};
//};

struct GraphNodeValue
{
    GraphNodeValue(std::type_index t, const std::string& n)
        : Type(t), TypeName(n)
    {
    }
    virtual void SetValuePtr(void*) = 0;
    virtual void* GetValuePtr() = 0;

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        out << Type.name() << " " << TypeName;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        // TODO: implement it using factory pattern
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValue& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, GraphNodeValue& n)
    {
        return n.Deserialize(in);
    }

protected:
    std::type_index Type;
    std::string TypeName;
};

struct GraphNodeValueFloat : public GraphNodeValue
{
    float Value;

public:
    GraphNodeValueFloat(float initValue)
        : GraphNodeValue(std::type_index(typeid(float)), "float"), Value(initValue)
    {}

    virtual void SetValuePtr(void* newValue) override { Value = *(float*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        GraphNodeValue::Serialize(out);
        out << " " << Value;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        GraphNodeValue::Deserialize(in);
        in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValueFloat& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, GraphNodeValueFloat& n) { return n.Deserialize(in); }
};

struct GraphNodeValueInt : public GraphNodeValue
{
    int Value;

public:
    GraphNodeValueInt(int initValue)
        : GraphNodeValue(std::type_index(typeid(int)), "int"), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(int*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        GraphNodeValue::Serialize(out);
        out << " " << Value;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        GraphNodeValue::Deserialize(in);
        in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValueInt& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, GraphNodeValueInt& n) { return n.Deserialize(in); }
};

struct GraphNodeValueFloat4x4 : public GraphNodeValue
{
    DirectX::XMFLOAT4X4 Value;

public:
    GraphNodeValueFloat4x4(DirectX::XMFLOAT4X4 initValue)
        : GraphNodeValue(std::type_index(typeid(int)), "float4x4"), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT4X4*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        GraphNodeValue::Serialize(out);
        LOG_ERROR("GraphNodeValueFloat4x4::Serialize NOT IMPLEMENTED");
        out << " " << "NOT IMPLEMENTED";
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        GraphNodeValue::Deserialize(in);
        LOG_ERROR("GraphNodeValueFloat4x4::Deserialize NOT IMPLEMENTED");
        //in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValueFloat4x4& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, GraphNodeValueFloat4x4& n) { return n.Deserialize(in); }
};

struct GraphNodeValueFloat4 : public GraphNodeValue
{
    DirectX::XMFLOAT4 Value;

public:
    GraphNodeValueFloat4(DirectX::XMFLOAT4 initValue)
        : GraphNodeValue(std::type_index(typeid(int)), "float4"), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT4*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        GraphNodeValue::Serialize(out);
        LOG_ERROR("GraphNodeValueFloat4::Serialize NOT IMPLEMENTED");
        out << " " << "NOT IMPLEMENTED";
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        GraphNodeValue::Deserialize(in);
        LOG_ERROR("GraphNodeValueFloat4::Deserialize NOT IMPLEMENTED");
        //in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValueFloat4& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, GraphNodeValueFloat4& n) { return n.Deserialize(in); }
};

struct GraphNodeValueFloat3 : public GraphNodeValue
{
    DirectX::XMFLOAT3 Value;

public:
    GraphNodeValueFloat3(DirectX::XMFLOAT3 initValue)
        : GraphNodeValue(std::type_index(typeid(int)), "float3"), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT3*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        GraphNodeValue::Serialize(out);
        LOG_ERROR("GraphNodeValueFloat3::Serialize NOT IMPLEMENTED");
        out << " " << "NOT IMPLEMENTED";
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        GraphNodeValue::Deserialize(in);
        LOG_ERROR("GraphNodeValueFloat3::Deserialize NOT IMPLEMENTED");
        //in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValueFloat3& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, GraphNodeValueFloat3& n) { return n.Deserialize(in); }
};

struct GraphNodeValueFloat2 : public GraphNodeValue
{
    DirectX::XMFLOAT2 Value;

public:
    GraphNodeValueFloat2(DirectX::XMFLOAT2 initValue)
        : GraphNodeValue(std::type_index(typeid(int)), "float2"), Value(initValue)
    {
    }

    virtual void SetValuePtr(void* newValue) override { Value = *(DirectX::XMFLOAT2*)newValue; }
    virtual void* GetValuePtr() override { return &Value; }

    virtual std::ostream& Serialize(std::ostream& out) const override
    {
        GraphNodeValue::Serialize(out);
        LOG_ERROR("GraphNodeValueFloat2::Serialize NOT IMPLEMENTED");
        out << " " << "NOT IMPLEMENTED";
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in) override
    {
        GraphNodeValue::Deserialize(in);
        LOG_ERROR("GraphNodeValueFloat2::Deserialize NOT IMPLEMENTED");
        //in >> Value;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValueFloat2& n) { return n.Serialize(out); }
    friend std::istream& operator>>(std::istream& in, GraphNodeValueFloat2& n) { return n.Deserialize(in); }
};
