#pragma once

#include "UiNode.h"

struct RenderTargetNode : UiNode
{
    explicit RenderTargetNode(Graph<Node>* graph, RenderTexture* renderTexture)
        : UiNode(graph, UiNodeType::RenderTarget), RenderTex(renderTexture), Input(INVALID_ID)
    {
    }
    
    RenderTexture* RenderTex;
    NodeId Input;

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node op(NodeType::RenderTarget);

        Input = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(op);

        ParentGraph->CreateEdge(Id, Input);
    }

    virtual void OnUpdate() override
    {
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(Input);
    }

    virtual void OnRender() override
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
            if (ParentGraph->GetNumEdgesFromNode(Input) == 0ull)
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
        Type = UiNodeType::RenderTarget;
        in >> Id >> Input;
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