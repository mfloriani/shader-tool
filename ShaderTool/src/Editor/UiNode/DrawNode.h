#pragma once

#include "UiNode.h"
#include "Rendering/Shader.h"

#include <vector>
#include <unordered_map>

const static int DEFAULT_SHADER_INDEX = 0;

struct ShaderBindingPin
{
    NodeId PinId{ INVALID_INDEX };
    ShaderBind Bind;
    void* Data{ nullptr };
};

struct DrawNode : UiNode
{
private:
    
public:
    NodeId ModelPin, ShaderPin, OutputPin;
    
    std::shared_ptr<NodeValueInt> ShaderNodeValue;
    std::shared_ptr<NodeValueInt> ModelNodeValue;
    std::shared_ptr<NodeValueInt> OutputNodeValue;

    std::vector<ShaderBindingPin>           ShaderBindingPins;
    std::unordered_map<std::string, size_t> ShaderBindingPinNameMap;
    std::unordered_map<NodeId, size_t>      ShaderBindingPinIdMap;

public:
    DrawNode(Graph* graph);
    virtual ~DrawNode();

    virtual void OnEvent(Event* e) override;
    virtual void OnCreate() override;
    virtual void OnLoad() override;
    virtual void OnDelete() override;
    virtual void OnRender() override;
    virtual void OnUpdate() override;
    virtual void OnEval() override;

    void CreateShaderBindingPins(int shaderIndex);
    void OnShaderLinkCreated(int from, int to);
    void OnShaderLinkDeleted(int from, int to);
    
    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << ModelPin << " " << ShaderPin << " " << OutputPin << " " << ShaderBindingPins.size();
        for (auto& bindPin : ShaderBindingPins)
        {
            out << "\n";
            out << bindPin.PinId << " " << bindPin.Bind;
        }
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Draw;
        size_t numShaderBindings;
        in >> Id >> ModelPin >> ShaderPin >> OutputPin >> numShaderBindings;
        if (numShaderBindings > 0)
        {
            ShaderBindingPins.reserve(numShaderBindings);
            for (int i = 0; i < numShaderBindings; ++i)
            {
                ShaderBindingPin pin;
                in >> pin.PinId >> pin.Bind;
                ShaderBindingPins.push_back(pin);
                size_t index = ShaderBindingPins.size() - 1ull;
                ShaderBindingPinNameMap[pin.Bind.VarName] = index;
                ShaderBindingPinIdMap[pin.PinId] = index;
            }
        }
        OnLoad();
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