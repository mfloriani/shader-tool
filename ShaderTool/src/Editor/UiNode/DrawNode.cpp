#include "pch.h"
#include "DrawNode.h"
#include "Defines.h"
#include "Rendering/ShaderManager.h"

using namespace DirectX;

DrawNode::DrawNode(Graph<Node>* graph)
    : UiNode(graph, UiNodeType::Draw), ModelPin(INVALID_ID), ShaderPin(INVALID_ID)
{
}

void DrawNode::OnCreate()
{
    const Node value(NodeType::Value, 0.f);
    const Node link(NodeType::Link, NOT_LINKED);
    const Node out(NodeType::Draw);

    ShaderPin = ParentGraph->CreateNode(link);
    ModelPin = ParentGraph->CreateNode(link);
    Id = ParentGraph->CreateNode(out);

    ParentGraph->CreateEdge(Id, ShaderPin);
    ParentGraph->CreateEdge(Id, ModelPin);
}

void DrawNode::OnUpdate(GameTimer& timer)
{
#if 1
    
    // Convert Spherical to Cartesian coordinates.
    _EyePos.x = _Radius * sinf(_Phi) * cosf(_Theta);
    _EyePos.z = _Radius * sinf(_Phi) * sinf(_Theta);
    _EyePos.y = _Radius * cosf(_Phi);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(_EyePos.x, _EyePos.y, _EyePos.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&_View, XMMatrixTranspose(view));


    auto aspectRatio = static_cast<float>(RENDER_TARGET_WIDTH) / RENDER_TARGET_HEIGHT;
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspectRatio, 1.0f, 1000.0f);
    XMStoreFloat4x4(&_Proj, XMMatrixTranspose(P));

    XMStoreFloat4x4(&_World, XMMatrixIdentity());

    // BOX ROTATION
    {
        //XMFLOAT3 Entity_Rotation = { 0.f, timer.TotalTime(), 0.f };
        //XMFLOAT3 Entity_Scale = { 10.f, 10.f, 10.f };
        //auto rotation = XMMatrixRotationY(Entity_Rotation.y);
        //auto scale = XMMatrixScaling(Entity_Scale.x, Entity_Scale.y, Entity_Scale.z);        
        //_World = XMMatrixMultiply(scale, rotation);

        //ObjectConstants objConstants;
        //XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

        //int objCBIndex = _Entity.Id; // TODO: handle multiple objects
        //currObjectCB->CopyData(objCBIndex, objConstants);
    }

#endif
}

void DrawNode::OnDelete() 
{
    ParentGraph->EraseNode(ModelPin);
    ParentGraph->EraseNode(ShaderPin);
}

void DrawNode::OnRender() 
{
    const float node_width = 200.0f;
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(81, 148, 204, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(81, 148, 204, 255));
    //ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(255, 255, 0, 255));
    ImNodes::BeginNode(Id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("DRAW");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(node_width, 0.f));

    {
        ImNodes::BeginInputAttribute(ShaderPin);
        const float label_width = ImGui::CalcTextSize("shader").x;
        ImGui::TextUnformatted("shader");
        if (ParentGraph->GetNumEdgesFromNode(ShaderPin) == 0ull)
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(node_width - label_width);
            ImGui::PopItemWidth();
        }
        ImNodes::EndInputAttribute();
    }

    ImGui::Spacing();

    {
        ImNodes::BeginInputAttribute(ModelPin);
        const float label_width = ImGui::CalcTextSize("model").x;
        ImGui::TextUnformatted("model");
        if (ParentGraph->GetNumEdgesFromNode(ModelPin) == 0ull)
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(node_width - label_width);
            ImGui::PopItemWidth();
        }
        ImNodes::EndInputAttribute();
    }

    // SHADER BINDINGS

    ImGui::Spacing();
    ImGui::Text("BINDS");
    ImGui::Spacing();

    if (Data.shader != NOT_LINKED)
    {
        int pinId = 100; // TODO: TEMPORARY
        auto& bindVars = ShaderManager::Get().GetShader(Data.shader)->GetBindingVars();
        for (auto& [id, bindVar] : bindVars)
        {
            for (auto& bind : bindVar)
            {
                std::string varNameType = bind.VarName + " (" + bind.VarTypeName + ")";

                ImNodes::BeginInputAttribute(pinId++);
                const float label_width = ImGui::CalcTextSize(varNameType.c_str()).x;
                ImGui::TextUnformatted(varNameType.c_str());
                ImNodes::EndInputAttribute();
                ImGui::Spacing();
            }
        }
    }

    ImGui::Spacing(); ImGui::Spacing();

    {
        ImNodes::BeginOutputAttribute(Id);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::TextUnformatted("output");
        ImNodes::EndOutputAttribute();
    }

    ImGui::Spacing();

    ImNodes::EndNode();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
}