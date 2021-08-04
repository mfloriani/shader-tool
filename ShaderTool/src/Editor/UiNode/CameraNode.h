#pragma once

#include "UiNode.h"

struct CameraNode : UiNode
{
private:
    ImVec4 TempCamera;

public:
    explicit CameraNode(Graph* graph)
        : UiNode(graph, UiNodeType::Camera)
    {
        //Camera = std::make_shared<NodeFloat3>(0.f);
    }

    //std::shared_ptr<NodeFloat3> Camera;

    NodeId ViewPin, ProjectionPin;

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node link(NodeType::Link);
        const Node op(NodeType::Camera);
        
        ViewPin = ParentGraph->CreateNode(link);
        ProjectionPin = ParentGraph->CreateNode(link);
        Id = ParentGraph->CreateNode(op);
        
        ParentGraph->CreateEdge(Id, ViewPin);
        ParentGraph->CreateEdge(Id, ProjectionPin);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        //SetPinValue(Id, 6.f);
        //Camera->value.x = TempCamera.x;
        //Camera->value.y = TempCamera.y;
        //Camera->value.z = TempCamera.z;
    }

    virtual void OnDelete() override
    {

    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;

        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("CAMERA");
        ImNodes::EndNodeTitleBar();
        
        ImNodes::BeginOutputAttribute(ViewPin);
        float label_width = ImGui::CalcTextSize("view").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("view");
        ImNodes::EndOutputAttribute();

        ImNodes::BeginOutputAttribute(ProjectionPin);
        label_width = ImGui::CalcTextSize("projection").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("projection");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        // TODO: implement this
        //out << " " << Camera->value.x << " " << Camera->value.y << " " << Camera->value.z;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Camera;
        // TODO: implement this
        //in >> Id >> TempCamera.x >> TempCamera.y >> TempCamera.z;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const CameraNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, CameraNode& n)
    {
        return n.Deserialize(in);
    }
};