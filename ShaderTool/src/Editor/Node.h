#pragma once

#include <fstream>

enum class UiNodeType
{
    add,
    multiply,
    output,
    sine,
    time,
};

enum class NodeType
{
    add,
    multiply,
    output,
    sine,
    time,
    value
};

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

struct UiNode
{
    UiNodeType type;
    int id;

    union
    {
        struct
        {
            int lhs, rhs;
        } add;

        struct
        {
            int lhs, rhs;
        } multiply;

        struct
        {
            int r, g, b;
        } output;

        struct
        {
            int input;
        } sine;
    };

    friend std::ostream& operator<<(std::ostream& out, const UiNode& n)
    {
        out << static_cast<int>(n.type)
            << " "
            << n.id;

        switch (n.type)
        {
        case UiNodeType::add:
        {
            out << " "
                << n.add.lhs
                << " "
                << n.add.rhs;
            break;
        }
        case UiNodeType::multiply:
        {
            out << " "
                << n.multiply.lhs
                << " "
                << n.multiply.rhs;
            break;
        }
        case UiNodeType::output:
        {
            out << " "
                << n.output.r
                << " "
                << n.output.g
                << " "
                << n.output.b;
            break;
        }
        case UiNodeType::sine:
        {
            out << " "
                << n.sine.input;
            break;
        }
        case UiNodeType::time:
            break;
        default:
            LOG_ERROR("Invalid UiNode::UiNodeType {0}", static_cast<int>(n.type));
            break;
        }

        return out;
    }

    friend std::istream& operator>>(std::istream& in, UiNode& uiNode)
    {
        int uinType;
        in >> uinType >> uiNode.id;

        uiNode.type = static_cast<UiNodeType>(uinType);

        switch (uiNode.type)
        {
        case UiNodeType::add:
        {
            in >> uiNode.add.lhs >> uiNode.add.rhs;
            //LOG_TRACE("{0} {1} {2} {3}", uiNode.type, uiNode.id, uiNode.add.lhs, uiNode.add.rhs);
            break;
        }
        case UiNodeType::multiply:
        {
            in >> uiNode.multiply.lhs >> uiNode.multiply.rhs;
            //LOG_TRACE("{0} {1} {2} {3}", uiNode.type, uiNode.id, uiNode.multiply.lhs, uiNode.multiply.rhs);
            break;
        }
        case UiNodeType::output:
        {
            in >> uiNode.output.r >> uiNode.output.g >> uiNode.output.b;
            //LOG_TRACE("{0} {1} {2} {3} {4}", uiNode.type, uiNode.id, uiNode.output.r, uiNode.output.g, uiNode.output.b);
            break;
        }
        case UiNodeType::sine:
        {
            in >> uiNode.sine.input;
            //LOG_TRACE("{0} {1} {2}", uiNode.type, uiNode.id, uiNode.sine.input);
            break;
        }
        case UiNodeType::time:
            //LOG_TRACE("{0} {1}", uiNode.type, uiNode.id);
            break;
        default:
            LOG_ERROR("Invalid UiNode::UiNodeType {0}", static_cast<int>(uiNode.type));
            break;
        }
        return in;
    }
};