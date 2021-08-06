#include "pch.h"
#include "DrawNode.h"
#include "Defines.h"
#include "Rendering/ShaderManager.h"
#include "Events/EventManager.h"
#include "Events/Event.h"

using namespace DirectX;

DrawNode::DrawNode(Graph* graph)
    : UiNode(graph, UiNodeType::Draw), ModelPin(INVALID_ID), ShaderPin(INVALID_ID), OutputPin(INVALID_ID)
{
    EVENT_MANAGER.Attach(this);

    ShaderNodeValue = std::make_shared<NodeValue<int>>();
    ShaderNodeValue->TypeName = "int";
    ShaderNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[ShaderNodeValue->TypeName];
    ShaderNodeValue->Data = INVALID_INDEX;

    ModelNodeValue = std::make_shared<NodeValue<int>>();
    ModelNodeValue->TypeName = "int";
    ModelNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[ModelNodeValue->TypeName];
    ModelNodeValue->Data = INVALID_INDEX;

    OutputNodeValue = std::make_shared<NodeValue<int>>();
    OutputNodeValue->TypeName = "int";
    OutputNodeValue->Num32BitValues = D3DUtil::HlslTypeMap[OutputNodeValue->TypeName];
    OutputNodeValue->Data = INVALID_INDEX;
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
    const Node idNode = Node(NodeType::Draw, NodeDirection::None);
    Id = ParentGraph->CreateNode(idNode);

    const Node indexNodeIn(NodeType::Int, NodeDirection::In);
    ShaderPin = ParentGraph->CreateNode(indexNodeIn);
    ModelPin = ParentGraph->CreateNode(indexNodeIn);

    const Node textureIndexNodeOut(NodeType::Int, NodeDirection::Out);
    OutputPin = ParentGraph->CreateNode(textureIndexNodeOut);

    ParentGraph->CreateEdge(Id, ShaderPin, EdgeType::Internal);
    ParentGraph->CreateEdge(Id, ModelPin, EdgeType::Internal);
    ParentGraph->CreateEdge(OutputPin, Id, EdgeType::Internal);

    StoreNodeValuePtr<int>(ShaderPin, ShaderNodeValue);
    StoreNodeValuePtr<int>(ModelPin, ModelNodeValue);
    StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
}

void DrawNode::OnLoad()
{
    StoreNodeValuePtr<int>(ShaderPin, ShaderNodeValue);
    StoreNodeValuePtr<int>(ModelPin, ModelNodeValue);
    StoreNodeValuePtr<int>(OutputPin, OutputNodeValue);
}

void DrawNode::OnDelete() 
{
    ParentGraph->EraseNode(ModelPin);
    ParentGraph->EraseNode(ShaderPin);
    ParentGraph->EraseNode(OutputPin);

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
    
    ImGui::Spacing(); ImGui::Spacing();

    {
        ImNodes::BeginOutputAttribute(OutputPin);
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

void DrawNode::OnUpdate()
{

}

void DrawNode::OnEval()
{
    bool isReadyToRender = true;

    if(!CopyFromLinkedSourceNodeValue<int>(ShaderPin, INVALID_INDEX))
        bool isReadyToRender = false;

    if (!CopyFromLinkedSourceNodeValue<int>(ModelPin, INVALID_INDEX))
        bool isReadyToRender = false;

    //XMFLOAT3 Entity_Rotation = { 0.f, timer.TotalTime(), 0.f };
    //XMFLOAT3 Entity_Scale = { 10.f, 10.f, 10.f };
    //auto rotation = XMMatrixRotationY(Entity_Rotation.y);
    //auto scale = XMMatrixScaling(Entity_Scale.x, Entity_Scale.y, Entity_Scale.z);
    //XMStoreFloat4x4(&_World, XMMatrixMultiply(scale, rotation));


    // SHADER BINDING PINS
    for (auto& bindPin : ShaderBindingPins)
    {
        size_t numNeighbors = ParentGraph->GetNumEdgesFromNode(bindPin.PinId);
        
        if (numNeighbors == 0ull) // no link
        {
            if (bindPin.Bind.VarTypeName == "float4x4")
            {
                auto value = GetNodeValuePtr<XMFLOAT4X4>(bindPin.PinId);
                value->Data = D3DUtil::Identity4x4();
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float4")
            {
                auto value = GetNodeValuePtr<XMFLOAT4>(bindPin.PinId);
                value->Data = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float3")
            {
                auto value = GetNodeValuePtr<XMFLOAT3>(bindPin.PinId);
                value->Data = XMFLOAT3(0.f, 0.f, 0.f);
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float2")
            {
                auto value = GetNodeValuePtr<XMFLOAT2>(bindPin.PinId);
                value->Data = XMFLOAT2(0.f, 0.f);
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float")
            {
                auto value = GetNodeValuePtr<float>(bindPin.PinId);
                value->Data = 0.f;
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "int")
            {
                auto value = GetNodeValuePtr<int>(bindPin.PinId);
                value->Data = 0;
                bindPin.Data = &value->Data;
            }
        }
        else if (numNeighbors == 1ull) // 1 link
        {
            int neighborId = *ParentGraph->GetNeighbors(bindPin.PinId).begin();
            if (bindPin.Bind.VarTypeName == "float4x4")
            {
                auto value = GetNodeValuePtr<XMFLOAT4X4>(bindPin.PinId);
                value->Data = GetNodeValuePtr<XMFLOAT4X4>(neighborId)->Data;
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float4")
            {
                auto value = GetNodeValuePtr<XMFLOAT4>(bindPin.PinId);
                value->Data = GetNodeValuePtr<XMFLOAT4>(neighborId)->Data;
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float3")
            {
                auto value = GetNodeValuePtr<XMFLOAT3>(bindPin.PinId);
                value->Data = GetNodeValuePtr<XMFLOAT3>(neighborId)->Data;
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float2")
            {
                auto value = GetNodeValuePtr<XMFLOAT2>(bindPin.PinId);
                value->Data = GetNodeValuePtr<XMFLOAT2>(neighborId)->Data;
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "float")
            {
                auto value = GetNodeValuePtr<float>(bindPin.PinId);
                value->Data = GetNodeValuePtr<float>(neighborId)->Data;
                bindPin.Data = &value->Data;
            }
            else if (bindPin.Bind.VarTypeName == "int")
            {
                auto value = GetNodeValuePtr<int>(bindPin.PinId);
                value->Data = GetNodeValuePtr<int>(neighborId)->Data;
                bindPin.Data = &value->Data;
            }
            else
            {
                LOG_CRITICAL("Unknown binding variable type {0}", bindPin.Bind.VarTypeName);
            }
        }
        else // more than 1 link
        {
            LOG_ERROR("Multiple links [{1}] at node {0}", ModelPin, numNeighbors);
            isReadyToRender = false;
        }
    }

    OutputNodeValue->Data = INVALID_INDEX;
    if(isReadyToRender) 
        OutputNodeValue->Data = 1; // only set valid index when all pins are linked

}

void DrawNode::CreateShaderBindings(Shader* shader)
{
    // get the constant buffer vars expected by the current shader
    for (auto& var : shader->GetBindingVars())
    {
        ShaderBindingPin bindPin;
        bindPin.Bind = var;

        NodeId pinId;
        if (var.VarTypeName == "float4x4")
        {
            const Node link(NodeType::Float4x4, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);
            auto value = std::make_shared<NodeValue<DirectX::XMFLOAT4X4>>();
            value->Data = D3DUtil::Identity4x4();
            StoreNodeValuePtr<DirectX::XMFLOAT4X4>(pinId, value);
            bindPin.Data = &value->Data;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float4")
        {
            const Node link(NodeType::Float4, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);
            auto value = std::make_shared<NodeValue<DirectX::XMFLOAT4>>();
            StoreNodeValuePtr<DirectX::XMFLOAT4>(pinId, value);
            bindPin.Data = &value->Data;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float3")
        {
            const Node link(NodeType::Float3, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);
            auto value = std::make_shared<NodeValue<DirectX::XMFLOAT3>>();
            StoreNodeValuePtr<DirectX::XMFLOAT3>(pinId, value);
            bindPin.Data = &value->Data;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float2")
        {
            const Node link(NodeType::Float2, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);
            auto value = std::make_shared<NodeValue<DirectX::XMFLOAT2>>();
            StoreNodeValuePtr<DirectX::XMFLOAT2>(pinId, value);
            bindPin.Data = &value->Data;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float")
        {
            const Node link(NodeType::Float, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);
            auto value = std::make_shared<NodeValue<float>>();
            StoreNodeValuePtr<float>(pinId, value);
            bindPin.Data = &value->Data;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "int")
        {
            const Node link(NodeType::Int, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);
            auto value = std::make_shared<NodeValue<int>>();
            StoreNodeValuePtr<int>(pinId, value);
            bindPin.Data = &value->Data;
            bindPin.PinId = pinId;
        }
        else
        {
            LOG_CRITICAL("Unknown binding variable type {0}", var.VarTypeName);

        }

        ShaderBindingPins.push_back(bindPin);
        ShaderBindingPinNameMap[var.VarName] = ShaderBindingPins.size() - 1;
        ShaderBindingPinIdMap[pinId] = ShaderBindingPins.size() - 1;
    }
}

// `from` and `to` are backwards
// `from` is the pin in this drawNode and `to` is the pin in the linked node
void DrawNode::OnShaderLinkCreated(int from, int to)
{
    LOG_TRACE("ShaderPin link created");
    
    int shaderIndex = GetNodeValuePtr<int>(to)->Data;
    auto shader = ShaderManager::Get().GetShader((size_t)shaderIndex);
    if (!shader)
    {
        LOG_ERROR("Failed to load shader binding vars! Invalid shader index {0}", shaderIndex);
        return;
    }
    CreateShaderBindings(shader);
}

void DrawNode::OnShaderLinkDeleted(int from, int to)
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
