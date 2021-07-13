#pragma once

#include <fstream>

enum class UiNodeType
{
    Add,
    Multiply,
    Draw,
    Sine,
    Time,
    RenderTarget,
    Primitive
};

enum class NodeType
{
    Value,
    Add,
    Multiply,
    Draw,
    Sine,
    Time,
    RenderTarget,
    Primitive
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
            int r, g, b, model;
        } draw;

        struct
        {
            int input;
        } sine;

        struct
        {
            int input;
        } renderTarget;

        struct
        {
            int input;
        } primitive;
    };

    friend std::ostream& operator<<(std::ostream& out, const UiNode& n)
    {
        out << static_cast<int>(n.type)
            << " "
            << n.id;

        switch (n.type)
        {
        case UiNodeType::Add:
        {
            out << " "
                << n.add.lhs
                << " "
                << n.add.rhs;
            break;
        }
        case UiNodeType::Multiply:
        {
            out << " "
                << n.multiply.lhs
                << " "
                << n.multiply.rhs;
            break;
        }
        case UiNodeType::Draw:
        {
            out << " "
                << n.draw.model
                << " "
                << n.draw.r
                << " "
                << n.draw.g
                << " "
                << n.draw.b;
            break;
        }
        case UiNodeType::Sine:
        {
            out << " "
                << n.sine.input;
            break;
        }

        case UiNodeType::Time:
            break;

        case UiNodeType::RenderTarget:

            // TODO: the input should point to the draw node
            out << " "
                << n.renderTarget.input;
            break;

        case UiNodeType::Primitive:

            out << " "
                << n.primitive.input;
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
        case UiNodeType::Add:
        {
            in >> uiNode.add.lhs >> uiNode.add.rhs;
            //LOG_TRACE("{0} {1} {2} {3}", uiNode.type, uiNode.id, uiNode.add.lhs, uiNode.add.rhs);
            break;
        }
        case UiNodeType::Multiply:
        {
            in >> uiNode.multiply.lhs >> uiNode.multiply.rhs;
            //LOG_TRACE("{0} {1} {2} {3}", uiNode.type, uiNode.id, uiNode.multiply.lhs, uiNode.multiply.rhs);
            break;
        }
        case UiNodeType::Draw:
        {
            in >> uiNode.draw.model >> uiNode.draw.r >> uiNode.draw.g >> uiNode.draw.b;
            //LOG_TRACE("{0} {1} {2} {3} {4}", uiNode.type, uiNode.id, uiNode.output.r, uiNode.output.g, uiNode.output.b);
            break;
        }
        case UiNodeType::Sine:
        {
            in >> uiNode.sine.input;
            //LOG_TRACE("{0} {1} {2}", uiNode.type, uiNode.id, uiNode.sine.input);
            break;
        }
        case UiNodeType::Time:
            //LOG_TRACE("{0} {1}", uiNode.type, uiNode.id);
            break;

        case UiNodeType::RenderTarget:
            //LOG_TRACE("{0} {1}", uiNode.type, uiNode.id);

            in >> uiNode.renderTarget.input;
            break;

        case UiNodeType::Primitive:
            //LOG_TRACE("{0} {1}", uiNode.type, uiNode.id);

            in >> uiNode.primitive.input;
            break;

        default:
            LOG_ERROR("Invalid UiNode::UiNodeType {0}", static_cast<int>(uiNode.type));
            break;
        }
        return in;
    }
};