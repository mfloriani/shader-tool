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
    std::shared_ptr<NodeValue<DirectX::XMFLOAT3>> EyeInNodeValue;
    std::shared_ptr<NodeValue<DirectX::XMFLOAT3>> EyeOutNodeValue;
    std::shared_ptr<NodeValue<DirectX::XMFLOAT4X4>> ViewNodeValue;
    std::shared_ptr<NodeValue<DirectX::XMFLOAT4X4>> ProjectionNodeValue;

public:
    explicit CameraNode(Graph* graph)
        : UiNode(graph, UiNodeType::Camera), 
        EyeInPin(INVALID_ID), EyeOutPin(INVALID_ID), ViewPin(INVALID_ID), ProjectionPin(INVALID_ID)
    {
        EyeInNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT3>>();
        EyeInNodeValue->TypeName = "float3";
        //EyeInNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[EyeInNodeValue->TypeName];
        EyeInNodeValue->Data = DirectX::XMFLOAT3(0.f, 0.f, 0.f);

        EyeOutNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT3>>();
        EyeOutNodeValue->TypeName = "float3";
        //EyeOutNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[EyeOutNodeValue->TypeName];
        EyeOutNodeValue->Data = DirectX::XMFLOAT3(0.f, 0.f, 0.f);

        ViewNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT4X4>>();
        ViewNodeValue->TypeName = "float4x4";
        //ViewNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[ViewNodeValue->TypeName];
        ViewNodeValue->Data = D3DUtil::Identity4x4();

        ProjectionNodeValue = std::make_shared<NodeValue<DirectX::XMFLOAT4X4>>();
        ProjectionNodeValue->TypeName = "float4x4";
        //ProjectionNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[ViewNodeValue->TypeName];
        ProjectionNodeValue->Data = D3DUtil::Identity4x4();
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

        StoreNodeValuePtr<DirectX::XMFLOAT3>(EyeInPin, EyeInNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT3>(EyeOutPin, EyeOutNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(ViewPin, ViewNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(ProjectionPin, ProjectionNodeValue);
    }

    virtual void OnLoad() override
    {
        StoreNodeValuePtr<DirectX::XMFLOAT3>(EyeInPin, EyeInNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT3>(EyeOutPin, EyeOutNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(ViewPin, ViewNodeValue);
        StoreNodeValuePtr<DirectX::XMFLOAT4X4>(ProjectionPin, ProjectionNodeValue);
    }
    
    virtual void OnUpdate() override
    {
        CopyFromLinkedSourceNodeValue<DirectX::XMFLOAT3>(EyeInPin, EyeInNodeValue->Data);
    }

    virtual void OnEval() override
    {
        // Convert Spherical to Cartesian coordinates.
        EyeOutNodeValue->Data.x = _Radius * sinf(_Phi) * cosf(_Theta);
        EyeOutNodeValue->Data.z = _Radius * sinf(_Phi) * sinf(_Theta);
        EyeOutNodeValue->Data.y = _Radius * cosf(_Phi);

        // Build the view matrix.
        DirectX::XMVECTOR pos = DirectX::XMVectorSet(EyeOutNodeValue->Data.x, EyeOutNodeValue->Data.y, EyeOutNodeValue->Data.z, 1.0f);
        DirectX::XMVECTOR target = DirectX::XMVectorZero();
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
        XMStoreFloat4x4(&ViewNodeValue->Data, DirectX::XMMatrixTranspose(view));

        // Build the projection matrix
        auto aspectRatio = static_cast<float>(RENDER_TARGET_WIDTH) / RENDER_TARGET_HEIGHT;
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio, 1.0f, 1000.0f);
        DirectX::XMStoreFloat4x4(&ProjectionNodeValue->Data, DirectX::XMMatrixTranspose(P));
    }

    virtual void OnDelete() override
    {
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
            ImGui::DragFloat3("##hidelabel0", &EyeInNodeValue->Data.x, 0.01f);
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
        out << " " << ViewPin << " " << ProjectionPin;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        Type = UiNodeType::Camera;
        in >> Id >> ViewPin >> ProjectionPin;
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