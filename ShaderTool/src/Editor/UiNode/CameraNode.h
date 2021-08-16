#pragma once

#include "UiNode.h"

struct CameraNode : UiNode
{
private:
    ImVec4 TempCamera;

    float _Theta = 1.5f * DirectX::XM_PI;
    float _Phi = DirectX::XM_PIDIV2 - 0.1f;
    float _Radius = 50.0f;
    

public:
    NodeId EyeInPin, EyeOutPin, ViewPin, ProjectionPin;
    
    std::shared_ptr<NodeValueFloat3>   EyeInNodeValue;
    std::shared_ptr<NodeValueFloat3>   EyeOutNodeValue;
    std::shared_ptr<NodeValueFloat4x4> ViewNodeValue;
    std::shared_ptr<NodeValueFloat4x4> ProjectionNodeValue;

public:
    explicit CameraNode(Graph* graph)
        : UiNode(graph, UiNodeType::Camera), 
        EyeInPin(INVALID_ID), EyeOutPin(INVALID_ID), ViewPin(INVALID_ID), ProjectionPin(INVALID_ID)
    {
        EyeInNodeValue = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3(0.f, 0.f, 0.f));
        EyeOutNodeValue = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3(0.f, 0.f, 0.f));
        ViewNodeValue = std::make_shared<NodeValueFloat4x4>(D3DUtil::Identity4x4());
        ProjectionNodeValue = std::make_shared<NodeValueFloat4x4>(D3DUtil::Identity4x4());
    }

    virtual void OnEvent(Event* e) override {}

    virtual void OnCreate() override
    {
        const Node idNode(NodeType::Camera, NodeDirection::None);
        Id = ParentGraph->CreateNode(idNode);

        const Node float3NodeIn(NodeType::Float3, NodeDirection::In);
        EyeInPin = ParentGraph->CreateNode(float3NodeIn);
        
        const Node float3NodeOut(NodeType::Float3, NodeDirection::Out);
        EyeOutPin = ParentGraph->CreateNode(float3NodeOut);

        const Node float4x4NodeOut(NodeType::Float4x4, NodeDirection::Out);
        ViewPin = ParentGraph->CreateNode(float4x4NodeOut);
        ProjectionPin = ParentGraph->CreateNode(float4x4NodeOut);
        
        ParentGraph->CreateEdge(Id, EyeInPin, EdgeType::Internal);
        ParentGraph->CreateEdge(EyeOutPin, Id, EdgeType::Internal);
        ParentGraph->CreateEdge(ViewPin, Id, EdgeType::Internal);
        ParentGraph->CreateEdge(ProjectionPin, Id, EdgeType::Internal);

        ParentGraph->StoreNodeValue(EyeInPin, EyeInNodeValue);
        ParentGraph->StoreNodeValue(EyeOutPin, EyeOutNodeValue);
        ParentGraph->StoreNodeValue(ViewPin, ViewNodeValue);
        ParentGraph->StoreNodeValue(ProjectionPin, ProjectionNodeValue);
    }

    virtual void OnLoad() override
    {
        EyeInNodeValue->Value = *(DirectX::XMFLOAT3*)ParentGraph->GetNodeValue(EyeInPin)->GetValuePtr();

        ParentGraph->StoreNodeValue(EyeInPin, EyeInNodeValue);
        ParentGraph->StoreNodeValue(EyeOutPin, EyeOutNodeValue);
        ParentGraph->StoreNodeValue(ViewPin, ViewNodeValue);
        ParentGraph->StoreNodeValue(ProjectionPin, ProjectionNodeValue);
    }
    
    virtual void OnUpdate() override
    {
        CopyValueFromLinkedSource(EyeInPin, (void*)&EyeInNodeValue->Value);
    }

    virtual void OnEval() override
    {
        // Convert Spherical to Cartesian coordinates.
        EyeOutNodeValue->Value.x = _Radius * sinf(_Phi) * cosf(_Theta);
        EyeOutNodeValue->Value.z = _Radius * sinf(_Phi) * sinf(_Theta);
        EyeOutNodeValue->Value.y = _Radius * cosf(_Phi);

        // Build the view matrix.
        DirectX::XMVECTOR pos = DirectX::XMVectorSet(EyeOutNodeValue->Value.x, EyeOutNodeValue->Value.y, EyeOutNodeValue->Value.z, 1.0f);
        DirectX::XMVECTOR target = DirectX::XMVectorZero();
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
        XMStoreFloat4x4(&ViewNodeValue->Value, DirectX::XMMatrixTranspose(view));

        // Build the projection matrix
        auto aspectRatio = static_cast<float>(RENDER_TARGET_WIDTH) / RENDER_TARGET_HEIGHT;
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio, 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&ProjectionNodeValue->Value, DirectX::XMMatrixTranspose(P));
    }

    virtual void OnDelete() override
    {
        ParentGraph->EraseNode(EyeInPin);
        ParentGraph->EraseNode(EyeOutPin);
        ParentGraph->EraseNode(ViewPin);
        ParentGraph->EraseNode(ProjectionPin);
    }

    virtual void OnRender() override
    {
        const float node_width = 200.f;

        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("CAMERA");
        ImNodes::EndNodeTitleBar();
        
        {
            ImNodes::BeginInputAttribute(EyeInPin);
            ImGui::TextUnformatted("eye"); ImGui::SameLine();
            ImGui::PushItemWidth(170.f);
            ImGui::DragFloat3("##hidelabel0", &EyeInNodeValue->Value.x, 0.01f);
            ImGui::PopItemWidth();
            ImNodes::EndInputAttribute();
        }
        ImGui::SameLine();

        {
            ImNodes::BeginOutputAttribute(EyeOutPin);
            ImGui::TextUnformatted(""); // needed to stay in the same line
            ImNodes::EndOutputAttribute();
        }

        {
            ImNodes::BeginOutputAttribute(ViewPin);
            const float label_width = ImGui::CalcTextSize("view").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("view");
            ImNodes::EndOutputAttribute();
        }

        {
            ImNodes::BeginOutputAttribute(ProjectionPin);
            const float label_width = ImGui::CalcTextSize("projection").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("projection");
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << EyeInPin << " " << EyeOutPin << " " << ViewPin << " " << ProjectionPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Camera;
        in >> Id >> EyeInPin >> EyeOutPin >> ViewPin >> ProjectionPin;
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