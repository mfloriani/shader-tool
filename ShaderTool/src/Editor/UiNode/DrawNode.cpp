#include "pch.h"
#include "DrawNode.h"
#include "Defines.h"
#include "Rendering/ShaderManager.h"
#include "Events/EventManager.h"
#include "Events/Event.h"

using namespace DirectX;

DrawNode::DrawNode(Graph* graph)
    : UiNode(graph, UiNodeType::Draw), ModelPin(INVALID_ID), ShaderPin(INVALID_ID)
{
    EVENT_MANAGER.Attach(this);
}

DrawNode::~DrawNode()
{
    EVENT_MANAGER.Detach(this);
}

void DrawNode::OnEvent(Event* e)
{
    if (dynamic_cast<LinkCreatedEvent*>(e))
    {
        auto lce = dynamic_cast<LinkCreatedEvent*>(e);
        LOG_TRACE("DrawNode::OnEvent -> LinkCreatedEvent {0} {1}", lce->from, lce->to);
        
        if(ShaderPin == lce->from)
            OnShaderLinkCreated(lce->from, lce->to);
    }
    
    if (dynamic_cast<LinkDeletedEvent*>(e))
    {
        auto lde = dynamic_cast<LinkDeletedEvent*>(e);
        LOG_TRACE("DrawNode::OnEvent -> LinkDeletedEvent {0} {1}", lde->from, lde->to);
        
        if (ShaderPin == lde->from)
            OnShaderLinkDeleted(lde->from, lde->to);
    }
}


void DrawNode::OnCreate()
{
    //const Node value(NodeType::Value, 0.f);
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


    XMFLOAT3 Entity_Rotation = { 0.f, timer.TotalTime(), 0.f };
    XMFLOAT3 Entity_Scale = { 10.f, 10.f, 10.f };
    auto rotation = XMMatrixRotationY(Entity_Rotation.y);
    auto scale = XMMatrixScaling(Entity_Scale.x, Entity_Scale.y, Entity_Scale.z);        
    
    XMStoreFloat4x4(&_World, XMMatrixMultiply(scale, rotation));

#endif



    

}

void DrawNode::OnDelete() 
{
    ParentGraph->EraseNode(ModelPin);
    ParentGraph->EraseNode(ShaderPin);

    for (auto& bind : ShaderBindingPins)
        ParentGraph->EraseNode(bind.PinId);
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
    
    if ((int)GetPinValue(ShaderPin) != NOT_LINKED)
    {
        for (auto& pin : ShaderBindingPins)
        {
            std::stringstream ss;
            ss << pin.Bind.VarName << " (" << pin.Bind.VarTypeName << ")";
            ImNodes::BeginInputAttribute(pin.PinId);
            const float label_width = ImGui::CalcTextSize(ss.str().c_str()).x;
            ImGui::TextUnformatted(ss.str().c_str());
            ImNodes::EndInputAttribute();
            ImGui::Spacing();
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



// `from` and `to` are backwards
// `from` is the pin in this drawNode and `to` is the pin in the linked node
void DrawNode::OnShaderLinkCreated(int from, int to)
{
    if (ShaderPin == from) 
    {
        LOG_TRACE("ShaderPin link created");

        size_t shaderIndex = (size_t)GetPinValue(to);
        auto shader = ShaderManager::Get().GetShader(shaderIndex);
        for (auto& var : shader->GetBindingVars())
        {
            std::string varNameType = var.VarName + " (" + var.VarTypeName + ")";

            const Node link(NodeType::Link, NOT_LINKED);
            NodeId pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId);
            
            ShaderBindingPin bindPin;
            bindPin.PinId = pinId;
            bindPin.Bind = var;
            bindPin.Data = nullptr;
            
            ShaderBindingPins.push_back(bindPin);
            ShaderBindingPinNameMap[var.VarName] = ShaderBindingPins.size()-1;
            ShaderBindingPinIdMap[pinId] = ShaderBindingPins.size()-1;
        }
    }
}

void DrawNode::OnShaderLinkDeleted(int from, int to)
{
    if (ShaderPin == from)
    {
        LOG_TRACE("ShaderPin link deleted");

        for (auto& bindPin : ShaderBindingPins)
        {
            LOG_TRACE("Deleting shader pin nodes {0} {1} {2}", bindPin.PinId, bindPin.Bind.VarName, bindPin.Bind.VarTypeName);
            ParentGraph->EraseNode(bindPin.PinId);
        }

        ShaderBindingPins.clear();
        ShaderBindingPinNameMap.clear();
        ShaderBindingPinIdMap.clear();
    }
}
