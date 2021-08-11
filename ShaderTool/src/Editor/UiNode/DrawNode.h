#pragma once

#include "UiNode.h"
#include "Rendering/Shader.h"

#include <vector>
#include <unordered_map>

const static int DEFAULT_SHADER_INDEX = 0;

struct ShaderBindingPin
{
    NodeId PinId;
    ShaderBind Bind;
    void* Data;
};

struct DrawNode : UiNode
{
private:
    
public:
    NodeId ModelPin, ShaderPin, OutputPin;
    
    std::shared_ptr<GraphNodeValueInt> ShaderNodeValue;
    std::shared_ptr<GraphNodeValueInt> ModelNodeValue;
    std::shared_ptr<GraphNodeValueInt> OutputNodeValue;

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

    void CreateShaderBindingPins(Shader* shader);
    void OnShaderLinkCreated(int from, int to);
    void OnShaderLinkDeleted(int from, int to);
        
    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << ModelPin << " " << ShaderPin << " " << OutputPin;
        out << " " << ShaderBindingPins.size();
        
        for (auto& bindPin : ShaderBindingPins)
            out << " " << bindPin.PinId << " " << bindPin.Bind.VarName << " " << bindPin.Bind.VarTypeName;

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
                in >> pin.PinId >> pin.Bind.VarName >> pin.Bind.VarTypeName;
                ShaderBindingPins.push_back(pin);
                ShaderBindingPinNameMap[pin.Bind.VarName] = ShaderBindingPins.size() - 1;
                ShaderBindingPinIdMap[pin.PinId] = ShaderBindingPins.size() - 1;
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