#pragma once

#include "UiNode.h"

struct ColorNode : UiNode
{
private:
    ImVec4 TempColor;

public:
    explicit ColorNode(Graph* graph)
        : UiNode(graph, UiNodeType::Color)
    {
        Color = std::make_shared<NodeFloat3>(0.f);
    }

    std::shared_ptr<NodeFloat3> Color;
    
    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node op(NodeType::Color);
        Id = ParentGraph->CreateNode(op);
        ParentGraph->StoreNodeValue(Id, Color);
    }

    virtual void OnUpdate(GameTimer& timer) override
    {
        //SetPinValue(Id, 6.f);
        Color->value.x = TempColor.x;
        Color->value.y = TempColor.y;
        Color->value.z = TempColor.z;
    }

    virtual void OnDelete() override
    {

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

        ImNodes::BeginOutputAttribute(Id);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Color->value.x << " " << Color->value.y << " " << Color->value.z;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Color;
        in >> Id >> TempColor.x >> TempColor.y >> TempColor.z;
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