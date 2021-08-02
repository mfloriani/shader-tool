#pragma once

#include "UiNode.h"

#include <vector>

const static int DEFAULT_SHADER_INDEX = 0;

//struct BindingPinNode
//{
//    NodeId Id;
//    std::string Name;
//    UINT ShaderRegister;
//    UINT Num32BitValues;
//};


struct DrawNode : UiNode
{
    DrawNode(Graph<Node>* graph);
    virtual ~DrawNode();

    NodeId ModelPin, ShaderPin;
    std::vector<NodeId> BindingPins;

    // TODO: move to Camera node 
    DirectX::XMFLOAT3 _EyePos = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT4X4 _View = D3DUtil::Identity4x4();
    DirectX::XMFLOAT4X4 _Proj = D3DUtil::Identity4x4();
    float _Theta = 1.5f * DirectX::XM_PI;
    float _Phi = DirectX::XM_PIDIV2 - 0.1f;
    float _Radius = 50.0f;
    //
    DirectX::XMFLOAT4X4 _World;

    virtual void OnEvent(Event* e) override;
    virtual void OnCreate() override;
    virtual void OnUpdate(GameTimer& timer) override;
    virtual void OnDelete() override;
    virtual void OnRender() override;

    void OnLinkCreated(int from, int to);
    void OnLinkDeleted(int from, int to);

    // TODO: rather useful methods
    // OnLoad()
    // OnNewLink()
    // OnLinkDelete()

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << ModelPin << " " << ShaderPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Draw;
        in >> Id >> ModelPin >> ShaderPin;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const DrawNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, DrawNode& n)
    {
        return n.Deserialize(in);
    }
};