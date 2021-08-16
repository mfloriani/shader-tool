#pragma once

#include "UiNode.h"

struct ColorNode : UiNode
{
private:
    ImVec4 TempColor;

public:
    NodeId OutputPin;
    std::shared_ptr<NodeValueFloat3> OutputNodeValue;

public:
    explicit ColorNode(Graph* graph)
        : UiNode(graph, UiNodeType::Color)
    {
        OutputNodeValue = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3(0.f, 0.f, 0.f));
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Color, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node colorNodeOut(NodeType::Float3, NodeDirection::Out);
        OutputPin = ParentGraph->CreateNode(colorNodeOut);

        ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnLoad() override
    {
        ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
    }

    virtual void OnUpdate() override
    {

    }

    virtual void OnEval() override
    {
        OutputNodeValue->Value = DirectX::XMFLOAT3(TempColor.x, TempColor.y, TempColor.z);
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(OutputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;

        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("COLOR PICKER");
        ImNodes::EndNodeTitleBar();

        ImGui::Indent(15);
        if (ImGui::ColorButton("##3c", TempColor, ImGuiColorEditFlags_NoBorder, ImVec2(80, 80)))
        {
            ImGui::OpenPopup("mypicker");
        }

        if (ImGui::BeginPopup("mypicker"))
        {
            ImGui::ColorPicker4("##picker", (float*)&TempColor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_PickerHueWheel);
            ImGui::EndPopup();
        }

        ImNodes::BeginOutputAttribute(OutputPin);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << OutputPin << " " << OutputNodeValue->Value.x << " " << OutputNodeValue->Value.y << " " << OutputNodeValue->Value.z;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Color;
        in >> Id >> OutputPin >> OutputNodeValue->Value.x >> OutputNodeValue->Value.y >> OutputNodeValue->Value.z;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const ColorNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, ColorNode& n)
    {
        return n.Deserialize(in);
    }
};