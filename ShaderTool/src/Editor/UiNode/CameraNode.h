#pragma once

#include "UiNode.h"

struct CameraNode : UiNode
{
private:
    ImVec4 TempCamera;

public:
    NodeId EyeInPin, EyeOutPin, TargetInPin, TargetOutPin, UpPin, RightPin, ViewPin, ProjectionPin;
    
    std::shared_ptr<NodeValueFloat3>   EyeInNodeValue;
    std::shared_ptr<NodeValueFloat3>   EyeOutNodeValue;
    std::shared_ptr<NodeValueFloat3>   TargetInNodeValue;
    std::shared_ptr<NodeValueFloat3>   TargetOutNodeValue;
    std::shared_ptr<NodeValueFloat3>   UpNodeValue;
    std::shared_ptr<NodeValueFloat3>   RightNodeValue;
    std::shared_ptr<NodeValueFloat4x4> ViewNodeValue;
    std::shared_ptr<NodeValueFloat4x4> ProjectionNodeValue;

public:
    explicit CameraNode(Graph* graph);

    virtual void OnEvent(Event* e) override {}
    virtual void OnCreate() override;    
    virtual void OnLoad() override;    
    virtual void OnUpdate() override;
    virtual void OnEval() override;
    virtual void OnDelete() override;
    virtual void OnRender() override;
    
    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << EyeInPin 
            << " " << EyeOutPin 
            << " " << TargetInPin
            << " " << TargetOutPin
            << " " << UpPin 
            << " " << RightPin 
            << " " << ViewPin 
            << " " << ProjectionPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Camera;
        in >> Id 
            >> EyeInPin 
            >> EyeOutPin 
            >> TargetInPin
            >> TargetOutPin
            >> UpPin 
            >> RightPin 
            >> ViewPin 
            >> ProjectionPin;
        OnLoad();
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const CameraNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, CameraNode& n)
    {
        return n.Deserialize(in);
    }
};