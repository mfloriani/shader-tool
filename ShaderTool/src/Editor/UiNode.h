#pragma once

#include "Graph.h"
#include "Node.h"

#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"

using UiNodeId = int;

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

struct UiNode
{
    UiNode() {}
    UiNode(UiNodeType type, UiNodeId id) : Type(type), Id(id) {}
    
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

struct AddNode : UiNode
{
    AddNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Left(-1), Right(-1)
    {
    }

    AddNode(UiNodeType type, UiNodeId id, NodeId lhs, NodeId rhs)
        : UiNode(type, id), Left(lhs), Right(rhs)
    {
    }

    NodeId Left, Right;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Left);
        graph.EraseNode(Right);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("ADD");
        ImNodes::EndNodeTitleBar();
        {
            ImNodes::BeginInputAttribute(Left);
            const float label_width = ImGui::CalcTextSize("left").x;
            ImGui::TextUnformatted("left");
            if (graph.GetNumEdgesFromNode(Left) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(Left).value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginInputAttribute(Right);
            const float label_width = ImGui::CalcTextSize("right").x;
            ImGui::TextUnformatted("right");
            if (graph.GetNumEdgesFromNode(Right) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(Right).value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginOutputAttribute(Id);
            const float label_width = ImGui::CalcTextSize("result").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("result");
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode base = static_cast<const UiNode&>(*this);
        base.Serialize(out);

        out << " "
            << Left
            << " "
            << Right;

        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Left >> Right;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const AddNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, AddNode& n)
    {
        return n.Deserialize(in);
    }
};

//union
//{
//    struct
//    {
//        int lhs, rhs;
//    } add;

//    struct
//    {
//        int lhs, rhs;
//    } multiply;

//    struct
//    {
//        int r, g, b, model, vs, ps;
//    } draw;

//    struct
//    {
//        int input;
//    } sine;

//    struct
//    {
//        int input;
//    } renderTarget;

//    struct
//    {
//        int input;
//    } primitive;
//};

//friend std::ostream& operator<<(std::ostream& out, const UiNode& n)
//{
//    out << static_cast<int>(n.type)
//        << " "
//        << n.id;

//    switch (n.type)
//    {
//    case UiNodeType::Add:
//    {
//        out << " "
//            << n.add.lhs
//            << " "
//            << n.add.rhs;
//        break;
//    }
//    case UiNodeType::Multiply:
//    {
//        out << " "
//            << n.multiply.lhs
//            << " "
//            << n.multiply.rhs;
//        break;
//    }
//    case UiNodeType::Draw:
//    {
//        out << " "
//            << n.draw.model
//            << " "
//            << n.draw.r
//            << " "
//            << n.draw.g
//            << " "
//            << n.draw.b;
//        break;
//    }
//    case UiNodeType::Sine:
//    {
//        out << " "
//            << n.sine.input;
//        break;
//    }

//    case UiNodeType::Time:
//        break;

//    case UiNodeType::RenderTarget:

//        out << " "
//            << n.renderTarget.input;
//        break;

//    case UiNodeType::Primitive:

//        out << " "
//            << n.primitive.input;
//        break;

//    default:
//        LOG_ERROR("Invalid UiNode::UiNodeType {0}", static_cast<int>(n.type));
//        break;
//    }

//    return out;
//}

//friend std::istream& operator>>(std::istream& in, UiNode& uiNode)
//{
//    int uinType;
//    in >> uinType >> uiNode.id;

//    uiNode.type = static_cast<UiNodeType>(uinType);

//    switch (uiNode.type)
//    {
//    case UiNodeType::Add:
//    {
//        in >> uiNode.add.lhs >> uiNode.add.rhs;
//        //LOG_TRACE("{0} {1} {2} {3}", uiNode.type, uiNode.id, uiNode.add.lhs, uiNode.add.rhs);
//        break;
//    }
//    case UiNodeType::Multiply:
//    {
//        in >> uiNode.multiply.lhs >> uiNode.multiply.rhs;
//        //LOG_TRACE("{0} {1} {2} {3}", uiNode.type, uiNode.id, uiNode.multiply.lhs, uiNode.multiply.rhs);
//        break;
//    }
//    case UiNodeType::Draw:
//    {
//        in >> uiNode.draw.model >> uiNode.draw.r >> uiNode.draw.g >> uiNode.draw.b;
//        //LOG_TRACE("{0} {1} {2} {3} {4}", uiNode.type, uiNode.id, uiNode.output.r, uiNode.output.g, uiNode.output.b);
//        break;
//    }
//    case UiNodeType::Sine:
//    {
//        in >> uiNode.sine.input;
//        //LOG_TRACE("{0} {1} {2}", uiNode.type, uiNode.id, uiNode.sine.input);
//        break;
//    }
//    case UiNodeType::Time:
//        //LOG_TRACE("{0} {1}", uiNode.type, uiNode.id);
//        break;

//    case UiNodeType::RenderTarget:
//        //LOG_TRACE("{0} {1}", uiNode.type, uiNode.id);

//        in >> uiNode.renderTarget.input;
//        break;

//    case UiNodeType::Primitive:
//        //LOG_TRACE("{0} {1}", uiNode.type, uiNode.id);

//        in >> uiNode.primitive.input;
//        break;

//    default:
//        LOG_ERROR("Invalid UiNode::UiNodeType {0}", static_cast<int>(uiNode.type));
//        break;
//    }
//    return in;
//}
//};