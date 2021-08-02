#pragma once

#include "UiNode.h"

struct RenderTargetNode : UiNode
{
    explicit RenderTargetNode(Graph<Node>* graph, RenderTexture* renderTexture)
        : UiNode(graph, UiNodeType::RenderTarget), RenderTex(renderTexture), Input(INVALID_ID), IsLinkedToDrawNode(false)
    {
        EVENT_MANAGER.Attach(this);
    }

    virtual ~RenderTargetNode()
    {
        EVENT_MANAGER.Detach(this);
    }
    
    RenderTexture* RenderTex;
    NodeId Input;

    bool IsLinkedToDrawNode;

    virtual void OnEvent(Event* e) override
    {
        if (dynamic_cast<LinkCreatedEvent*>(e))
        {
            auto lce = dynamic_cast<LinkCreatedEvent*>(e);
            LOG_TRACE("RenderTargetNode::OnEvent -> LinkCreatedEvent {0} {1}", lce->from, lce->to);

            if (Input == lce->from)
                OnDrawLinkCreated(lce->from, lce->to);
        }

        if (dynamic_cast<LinkDeletedEvent*>(e))
        {
            auto lde = dynamic_cast<LinkDeletedEvent*>(e);
            LOG_TRACE("RenderTargetNode::OnEvent -> LinkDeletedEvent {0} {1}", lde->from, lde->to);

            if (Input == lde->from)
                OnDrawLinkDeleted(lde->from, lde->to);
        }
    }

    void OnDrawLinkCreated(int from, int to)
    {
        IsLinkedToDrawNode = true;
    }

    void OnDrawLinkDeleted(int from, int to)
    {
        IsLinkedToDrawNode = false;
    }

    virtual void OnCreate() override
    {
        const Node value(NodeType::Value, 0.f);
        const Node op(NodeType::RenderTarget);

        Input = ParentGraph->CreateNode(value);
        Id = ParentGraph->CreateNode(op);

        ParentGraph->CreateEdge(Id, Input);
    }

    virtual void OnUpdate(GameTimer& timer) override
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
            
            if(IsLinkedToDrawNode)
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