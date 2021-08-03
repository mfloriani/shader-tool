#pragma once

#include "UiNode.h"

#include <vector>
#include <unordered_map>

const static int DEFAULT_SHADER_INDEX = 0;

struct ShaderBindingPin
{
    NodeId PinId;
    std::string VarName;
    std::string VarTypeName;
};

struct DrawNode : UiNode
{
    DrawNode(Graph<Node>* graph);
    virtual ~DrawNode();

    NodeId ModelPin, ShaderPin;
    
    std::vector<ShaderBindingPin> ShaderBindingPins;
    std::unordered_map<std::string, size_t> ShaderBindingPinsMap;

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

    void OnShaderLinkCreated(int from, int to);
    void OnShaderLinkDeleted(int from, int to);

    // TODO: rather useful methods
    // OnLoad()
    
    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << ModelPin << " " << ShaderPin;
        out << " " << ShaderBindingPins.size();
        for (auto& bind : ShaderBindingPins)
            out << " " << bind.PinId << " " << bind.VarName << " " << bind.VarTypeName;

        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Draw;
        size_t numShaderBindings;
        in >> Id >> ModelPin >> ShaderPin >> numShaderBindings;
        if (numShaderBindings > 0)
        {
            ShaderBindingPins.reserve(numShaderBindings);
            for (int i = 0; i < numShaderBindings; ++i)
            {
                ShaderBindingPin pin;
                in >> pin.PinId >> pin.VarName >> pin.VarTypeName;
                ShaderBindingPins.push_back(pin);
                ShaderBindingPinsMap[pin.VarName] = ShaderBindingPins.size() - 1;
            }
        }
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