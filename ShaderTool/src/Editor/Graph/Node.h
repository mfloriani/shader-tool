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

template<class T>
struct NodeValue
{
    std::string TypeName{""};
    //UINT Num32BitValues{ 0 };
    T Data{};
};

struct GraphNodeValue
{
    GraphNodeValue(std::type_index t, const std::string& n)
        : Type(t), TypeName(n)
    {
    }
    virtual void SetValue(void*) = 0;
    virtual void* GetValue() = 0;

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        out << Type.name() << " " << TypeName;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
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

    virtual void SetValue(void* newValue) override { Value = *(float*)newValue; }
    virtual void* GetValue() override { return &Value; }

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

    virtual void SetValue(void* newValue) override { Value = *(int*)newValue; }
    virtual void* GetValue() override { return &Value; }

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

//class GraphNodeValueStorage
//{
//public:
//    //void StoreNodeValuePtr(std::type_index typeIndex, std::shared_ptr<GraphNodeValue>& valuePtr)
//    void StoreNodeValuePtr(int nodeId, std::shared_ptr<GraphNodeValue>& valuePtr)
//    {
//        //_TypeValueMap[typeIndex].push_back(valuePtr);
//        _NodeIdValueMap[nodeId] = valuePtr;
//    }
//
//    std::shared_ptr<GraphNodeValue>& GetNodeValuePtr(int nodeId)
//    {
//        return _NodeIdValueMap[nodeId];
//    }
//
//    virtual std::ostream& Serialize(std::ostream& out) const
//    {
//        out << _NodeIdValueMap.size() << "\n";
//        for (auto& [nodeId, nodeValue] : _NodeIdValueMap)
//        {
//            out << "nv " << nodeId << " " << *nodeValue.get() << "\n";
//        }
//        return out;
//    }
//
//    virtual std::istream& Deserialize(std::istream& in)
//    {
//        return in;
//    }
//
//    friend std::ostream& operator<<(std::ostream& out, const GraphNodeValueStorage& n)
//    {
//        return n.Serialize(out);
//    }
//
//    friend std::istream& operator>>(std::istream& in, GraphNodeValueStorage& n)
//    {
//        return n.Deserialize(in);
//    }
//
//private:
//    std::unordered_map<int, std::shared_ptr<GraphNodeValue>> _NodeIdValueMap;
//
//    //std::unordered_map<std::string, std::type_index> _NameTypeMap;
//    //std::unordered_map<std::type_index, std::vector<std::shared_ptr<GraphNodeValue>>> _TypeValueMap;
//};