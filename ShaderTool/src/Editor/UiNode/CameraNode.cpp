#include "pch.h"
#include "CameraNode.h"

using namespace DirectX;
using namespace D3DUtil;

CameraNode::CameraNode(Graph* graph)
    : UiNode(graph, UiNodeType::Camera),
    EyeInPin(INVALID_ID), EyeOutPin(INVALID_ID), ViewPin(INVALID_ID), ProjectionPin(INVALID_ID)
{
    EyeInNodeValue = std::make_shared<NodeValueFloat3>(XMFLOAT3(0.f, 0.f, -3.f));
    EyeOutNodeValue = std::make_shared<NodeValueFloat3>(XMFLOAT3(0.f, 0.f, 0.f));
    TargetInNodeValue = std::make_shared<NodeValueFloat3>(XMFLOAT3(0.f, 0.f, 1.f));
    TargetOutNodeValue = std::make_shared<NodeValueFloat3>(XMFLOAT3(0.f, 0.f, 1.f));
    UpNodeValue = std::make_shared<NodeValueFloat3>(XMFLOAT3(0.f, 1.f, 0.f));
    RightNodeValue = std::make_shared<NodeValueFloat3>(XMFLOAT3(1.f, 0.f, 0.f));
    ViewNodeValue = std::make_shared<NodeValueFloat4x4>(Identity4x4());
    ProjectionNodeValue = std::make_shared<NodeValueFloat4x4>(Identity4x4());
}

void CameraNode::OnCreate()
{
    const Node idNode(NodeType::Camera, NodeDirection::None);
    Id = ParentGraph->CreateNode(idNode);

    const Node float3NodeIn(NodeType::Float3, NodeDirection::In);
    EyeInPin = ParentGraph->CreateNode(float3NodeIn);
    TargetInPin = ParentGraph->CreateNode(float3NodeIn);

    const Node float3NodeOut(NodeType::Float3, NodeDirection::Out);
    EyeOutPin = ParentGraph->CreateNode(float3NodeOut);
    TargetOutPin = ParentGraph->CreateNode(float3NodeOut);
    UpPin = ParentGraph->CreateNode(float3NodeOut);
    RightPin = ParentGraph->CreateNode(float3NodeOut);

    const Node float4x4NodeOut(NodeType::Float4x4, NodeDirection::Out);
    ViewPin = ParentGraph->CreateNode(float4x4NodeOut);
    ProjectionPin = ParentGraph->CreateNode(float4x4NodeOut);

    ParentGraph->CreateEdge(Id, EyeInPin, EdgeType::Internal);
    ParentGraph->CreateEdge(Id, TargetInPin, EdgeType::Internal);
    ParentGraph->CreateEdge(EyeOutPin, Id, EdgeType::Internal);
    ParentGraph->CreateEdge(TargetOutPin, Id, EdgeType::Internal);
    ParentGraph->CreateEdge(UpPin, Id, EdgeType::Internal);
    ParentGraph->CreateEdge(RightPin, Id, EdgeType::Internal);
    ParentGraph->CreateEdge(ViewPin, Id, EdgeType::Internal);
    ParentGraph->CreateEdge(ProjectionPin, Id, EdgeType::Internal);

    ParentGraph->StoreNodeValue(EyeInPin, EyeInNodeValue);
    ParentGraph->StoreNodeValue(EyeOutPin, EyeOutNodeValue);
    ParentGraph->StoreNodeValue(TargetInPin, TargetInNodeValue);
    ParentGraph->StoreNodeValue(TargetOutPin, TargetOutNodeValue);
    ParentGraph->StoreNodeValue(UpPin, UpNodeValue);
    ParentGraph->StoreNodeValue(RightPin, RightNodeValue);
    ParentGraph->StoreNodeValue(ViewPin, ViewNodeValue);
    ParentGraph->StoreNodeValue(ProjectionPin, ProjectionNodeValue);
}

void CameraNode::OnLoad()
{
    EyeInNodeValue->Value = *(XMFLOAT3*)ParentGraph->GetNodeValue(EyeInPin)->GetValuePtr();
    TargetInNodeValue->Value = *(XMFLOAT3*)ParentGraph->GetNodeValue(TargetInPin)->GetValuePtr();

    ParentGraph->StoreNodeValue(EyeInPin, EyeInNodeValue);
    ParentGraph->StoreNodeValue(EyeOutPin, EyeOutNodeValue);
    ParentGraph->StoreNodeValue(TargetInPin, TargetInNodeValue);
    ParentGraph->StoreNodeValue(TargetOutPin, TargetOutNodeValue);
    ParentGraph->StoreNodeValue(UpPin, UpNodeValue);
    ParentGraph->StoreNodeValue(RightPin, RightNodeValue);
    ParentGraph->StoreNodeValue(ViewPin, ViewNodeValue);
    ParentGraph->StoreNodeValue(ProjectionPin, ProjectionNodeValue);
}

void CameraNode::OnUpdate()
{
    CopyValueFromLinkedSource(EyeInPin, (void*)&EyeInNodeValue->Value);
    CopyValueFromLinkedSource(TargetInPin, (void*)&TargetInNodeValue->Value);
}

void CameraNode::OnEval()
{
    // Convert Spherical to Cartesian coordinates.        
    //EyeOutNodeValue->Value.x = _Radius * sinf(_Phi) * cosf(_Theta);
    //EyeOutNodeValue->Value.z = _Radius * sinf(_Phi) * sinf(_Theta);
    //EyeOutNodeValue->Value.y = _Radius * cosf(_Phi);

    EyeOutNodeValue->Value = EyeInNodeValue->Value;
    TargetOutNodeValue->Value = TargetInNodeValue->Value;

    // Build the view matrix.
    //XMVECTOR pos = XMLoadFloat3(&EyeOutNodeValue->Value);
    //XMVECTOR target = XMLoadFloat3(&TargetOutNodeValue->Value);
    //if (XMVector3Equal(target, XMVectorZero()))
    //{
    //    target = XMVectorSet(0.f, 0.f, 0.1f, 0.f);
    //    XMStoreFloat3(&TargetInNodeValue->Value, target);
    //    XMStoreFloat3(&TargetOutNodeValue->Value, target);
    //}
    //XMVECTOR up = XMLoadFloat3(&UpNodeValue->Value);
    //XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    //XMStoreFloat4x4(&ViewNodeValue->Value, XMMatrixTranspose(view));



    XMVECTOR R = XMLoadFloat3(&RightNodeValue->Value);
    XMVECTOR U = XMLoadFloat3(&UpNodeValue->Value);
    XMVECTOR L = XMLoadFloat3(&TargetOutNodeValue->Value);
    XMVECTOR P = XMLoadFloat3(&EyeOutNodeValue->Value);

    // Keep camera's axes orthogonal to each other and of unit length.
    L = XMVector3Normalize(L);
    U = XMVector3Normalize(XMVector3Cross(L, R));

    // U, L already ortho-normal, so no need to normalize cross product.
    R = XMVector3Cross(U, L);

    // Fill in the view matrix entries.
    float x = -XMVectorGetX(XMVector3Dot(P, R));
    float y = -XMVectorGetX(XMVector3Dot(P, U));
    float z = -XMVectorGetX(XMVector3Dot(P, L));

    XMStoreFloat3(&RightNodeValue->Value, R);
    XMStoreFloat3(&UpNodeValue->Value, U);
    XMStoreFloat3(&TargetOutNodeValue->Value, L);

    ViewNodeValue->Value(0, 0) = RightNodeValue->Value.x;
    ViewNodeValue->Value(1, 0) = RightNodeValue->Value.y;
    ViewNodeValue->Value(2, 0) = RightNodeValue->Value.z;
    ViewNodeValue->Value(3, 0) = x;

    ViewNodeValue->Value(0, 1) = UpNodeValue->Value.x;
    ViewNodeValue->Value(1, 1) = UpNodeValue->Value.y;
    ViewNodeValue->Value(2, 1) = UpNodeValue->Value.z;
    ViewNodeValue->Value(3, 1) = y;

    ViewNodeValue->Value(0, 2) = TargetOutNodeValue->Value.x;
    ViewNodeValue->Value(1, 2) = TargetOutNodeValue->Value.y;
    ViewNodeValue->Value(2, 2) = TargetOutNodeValue->Value.z;
    ViewNodeValue->Value(3, 2) = z;

    ViewNodeValue->Value(0, 3) = 0.0f;
    ViewNodeValue->Value(1, 3) = 0.0f;
    ViewNodeValue->Value(2, 3) = 0.0f;
    ViewNodeValue->Value(3, 3) = 1.0f;

    XMMATRIX view = XMMatrixTranspose( XMLoadFloat4x4(&ViewNodeValue->Value) );
    XMStoreFloat4x4(&ViewNodeValue->Value, view);

    // Build the projection matrix
    auto aspectRatio = static_cast<float>(RENDER_TARGET_WIDTH) / RENDER_TARGET_HEIGHT;
    XMMATRIX projection = XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 1.0f, 100.0f);
    XMStoreFloat4x4(&ProjectionNodeValue->Value, XMMatrixTranspose(projection));
}

void CameraNode::OnDelete()
{
    ParentGraph->EraseNode(EyeInPin);
    ParentGraph->EraseNode(EyeOutPin);
    ParentGraph->EraseNode(TargetInPin);
    ParentGraph->EraseNode(TargetOutPin);
    ParentGraph->EraseNode(UpPin);
    ParentGraph->EraseNode(RightPin);
    ParentGraph->EraseNode(ViewPin);
    ParentGraph->EraseNode(ProjectionPin);
}

void CameraNode::OnRender()
{
    const float node_width = 190.f;
    const float vec3_width = 170.f;
    const float indent_width = 7.f;

    ImNodes::BeginNode(Id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("CAMERA");
    ImNodes::EndNodeTitleBar();

    {
        ImNodes::BeginInputAttribute(EyeInPin);
        //ImGui::TextUnformatted("eye"); ImGui::SameLine();
        ImGui::PushItemWidth(vec3_width);
        ImGui::DragFloat3("##hidelabel1", &EyeInNodeValue->Value.x, 0.01f);
        ImGui::PopItemWidth();
        ImNodes::EndInputAttribute();

        ImGui::SameLine();

        ImNodes::BeginOutputAttribute(EyeOutPin);
        const float label_width = ImGui::CalcTextSize("eye").x;
        ImGui::Indent(node_width - label_width - vec3_width);
        ImGui::TextUnformatted("eye");
        //ImGui::TextUnformatted(""); // need this to stay in the same line of the input
        ImNodes::EndOutputAttribute();
    }

    {
        ImNodes::BeginInputAttribute(TargetInPin);
        //ImGui::TextUnformatted("target"); ImGui::SameLine();
        ImGui::PushItemWidth(vec3_width);
        ImGui::DragFloat3("##hidelabel2", &TargetInNodeValue->Value.x, 0.01f);
        ImGui::PopItemWidth();
        ImNodes::EndInputAttribute();

        ImGui::SameLine();

        ImNodes::BeginOutputAttribute(TargetOutPin);
        const float label_width = ImGui::CalcTextSize("at").x;
        ImGui::Indent(node_width - label_width - vec3_width);
        ImGui::TextUnformatted("at");
        //ImGui::TextUnformatted(""); // need this to stay in the same line of the input
        ImNodes::EndOutputAttribute();
    }

    {
        ImNodes::BeginOutputAttribute(UpPin);
        const float label_width = ImGui::CalcTextSize("up").x;
        ImGui::Indent(node_width - label_width + indent_width);
        ImGui::TextUnformatted("up");
        ImNodes::EndOutputAttribute();
    }

    {
        ImNodes::BeginOutputAttribute(RightPin);
        const float label_width = ImGui::CalcTextSize("right").x;
        ImGui::Indent(node_width - label_width + indent_width);
        ImGui::TextUnformatted("right");
        ImNodes::EndOutputAttribute();
    }

    {
        ImNodes::BeginOutputAttribute(ViewPin);
        const float label_width = ImGui::CalcTextSize("view").x;
        ImGui::Indent(node_width - label_width + indent_width);
        ImGui::TextUnformatted("view");
        ImNodes::EndOutputAttribute();
    }

    {
        ImNodes::BeginOutputAttribute(ProjectionPin);
        const float label_width = ImGui::CalcTextSize("projection").x;
        ImGui::Indent(node_width - label_width + indent_width);
        ImGui::TextUnformatted("projection");
        ImNodes::EndOutputAttribute();
    }

    ImNodes::EndNode();
}
