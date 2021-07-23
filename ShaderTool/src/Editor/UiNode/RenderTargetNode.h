#pragma once

#include "UiNode.h"

struct RenderTargetNode : UiNode
{
    explicit RenderTargetNode(UiNodeType type, UiNodeId id, RenderTexture* renderTexture)
        : UiNode(type, id), RenderTex(renderTexture), Input(INVALID_ID)
    {
    }

    explicit RenderTargetNode(UiNodeType type, UiNodeId id, RenderTexture* renderTexture, NodeId input)
        : UiNode(type, id), RenderTex(renderTexture), Input(input)
    {
    }

    RenderTexture* RenderTex;
    NodeId Input;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Input);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("RENDER TARGET");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(Input);
            const float label_width = ImGui::CalcTextSize("input").x;
            ImGui::TextUnformatted("input");
            if (graph.GetNumEdgesFromNode(Input) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();

            static int w = 256;
            static int h = 256;
            //if(_RenderTargetReady)
            ImGui::Image((ImTextureID)RenderTex->SRV().ptr, ImVec2((float)w, (float)h));
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Input;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Input;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const RenderTargetNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, RenderTargetNode& n)
    {
        return n.Deserialize(in);
    }
};