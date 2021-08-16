#pragma once

#include "UiNode.h"

struct RenderTargetNode : UiNode
{
private:
    RenderTexture* _RenderTex;
    bool _IsLinkedToDrawNode;

public:
    NodeId InputPin;
    std::shared_ptr<NodeValueInt> InputNodeValue;

public:
    explicit RenderTargetNode(Graph* graph, RenderTexture* renderTexture)
        : UiNode(graph, UiNodeType::RenderTarget), _RenderTex(renderTexture), InputPin(INVALID_ID), _IsLinkedToDrawNode(false)
    {
        EVENT_MANAGER.Attach(this);
        InputNodeValue = std::make_shared<NodeValueInt>(INVALID_INDEX);
    }

    virtual ~RenderTargetNode()
    {
        EVENT_MANAGER.Detach(this);
    }
    
    virtual void OnEvent(Event* e) override
    {
        if (dynamic_cast<LinkCreatedEvent*>(e))
        {
            auto lce = dynamic_cast<LinkCreatedEvent*>(e);
            LOG_TRACE("RenderTargetNode::OnEvent -> LinkCreatedEvent {0} {1}", lce->from, lce->to);

            if (InputPin == lce->from)
                OnDrawLinkCreated(lce->from, lce->to);
        }

        if (dynamic_cast<LinkDeletedEvent*>(e))
        {
            auto lde = dynamic_cast<LinkDeletedEvent*>(e);
            LOG_TRACE("RenderTargetNode::OnEvent -> LinkDeletedEvent {0} {1}", lde->from, lde->to);

            if (InputPin == lde->from)
                OnDrawLinkDeleted(lde->from, lde->to);
        }
    }

    void OnDrawLinkCreated(int from, int to)
    {
        _IsLinkedToDrawNode = true;
    }

    void OnDrawLinkDeleted(int from, int to)
    {
        _IsLinkedToDrawNode = false;
    }

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::RenderTarget, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node inputNode(NodeType::Int, NodeDirection::In);
        InputPin = ParentGraph->CreateNode(inputNode);

        ParentGraph->CreateEdge(Id, InputPin, EdgeType::Internal);
        ParentGraph->StoreNodeValue(InputPin, InputNodeValue);
    }

    virtual void OnLoad() override
    {
        ParentGraph->StoreNodeValue(InputPin, InputNodeValue);
        if (ParentGraph->GetNumEdgesFromNode(InputPin) > 0ull)
            _IsLinkedToDrawNode = true;
    }

    virtual void OnUpdate() override
    {

    }
    
    virtual void OnEval() override
    {

    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(InputPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("RENDER TARGET");
        ImNodes::EndNodeTitleBar();

        {
            ImNodes::BeginInputAttribute(InputPin);
            const float label_width = ImGui::CalcTextSize("input").x;
            ImGui::TextUnformatted("input");
            ImNodes::EndInputAttribute();

            static int w = 256;
            static int h = 256;
            
            if(_IsLinkedToDrawNode)
                ImGui::Image((ImTextureID)_RenderTex->SRV().ptr, ImVec2((float)w, (float)h));
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << InputPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::RenderTarget;
        in >> Id >> InputPin;
        OnLoad();
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