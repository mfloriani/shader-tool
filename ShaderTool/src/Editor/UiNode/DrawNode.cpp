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
    
    ShaderNodeValue = std::make_shared<NodeValueInt>(INVALID_INDEX);
    ModelNodeValue = std::make_shared<NodeValueInt>(INVALID_INDEX);
    OutputNodeValue = std::make_shared<NodeValueInt>(INVALID_INDEX);
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
        //LOG_TRACE("DrawNode::OnEvent -> LinkCreatedEvent {0} {1}", lce->from, lce->to);
        
        if(ShaderPin == lce->from)
            OnShaderLinkCreate(lce->from, lce->to);
    }
    
    if (dynamic_cast<LinkDeletedEvent*>(e))
    {
        auto lde = dynamic_cast<LinkDeletedEvent*>(e);
        //LOG_TRACE("DrawNode::OnEvent -> LinkDeletedEvent {0} {1}", lde->from, lde->to);
        
        if (ShaderPin == lde->from)
            OnShaderLinkDelete(lde->from, lde->to);
    }

    if (dynamic_cast<ShaderUpdatedEvent*>(e))
    {
        auto sue = dynamic_cast<ShaderUpdatedEvent*>(e);
        //LOG_TRACE("DrawNode::OnEvent -> ShaderUpdatedEvent");

        if(ParentGraph->GetNumEdgesFromNode(ShaderPin) > 0ull)
            OnLinkedShaderUpdate(sue->index);
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

    ParentGraph->StoreNodeValue(ShaderPin, ShaderNodeValue);
    ParentGraph->StoreNodeValue(ModelPin, ModelNodeValue);
    ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
}

void DrawNode::OnLoad()
{
    ParentGraph->StoreNodeValue(ShaderPin, ShaderNodeValue);
    ParentGraph->StoreNodeValue(ModelPin, ModelNodeValue);
    ParentGraph->StoreNodeValue(OutputPin, OutputNodeValue);
}

void DrawNode::OnDelete() 
{
    ParentGraph->EraseNode(ModelPin);
    ParentGraph->EraseNode(ShaderPin);
    ParentGraph->EraseNode(OutputPin);

    ClearShaderBindings();
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

    if (!CopyValueFromLinkedSource(ShaderPin, (void*)&INVALID_INDEX))
        isReadyToRender = false;

    if (!CopyValueFromLinkedSource(ModelPin, (void*)&INVALID_INDEX))
        isReadyToRender = false;

    // SHADER BINDING PINS
    for (auto& bindPin : ShaderBindingPins)
    {
        if (bindPin.Bind.VarTypeName == "float4x4")
        {
            CopyValueFromLinkedSource(bindPin.PinId, (void*)&D3DUtil::Identity4x4());
            bindPin.Data = ParentGraph->GetNodeValue(bindPin.PinId)->GetValuePtr();
        }
        else if (bindPin.Bind.VarTypeName == "float4")
        {
            auto defaultValue = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
            CopyValueFromLinkedSource(bindPin.PinId, (void*)&defaultValue);
            bindPin.Data = ParentGraph->GetNodeValue(bindPin.PinId)->GetValuePtr();
        }
        else if (bindPin.Bind.VarTypeName == "float3")
        {
            auto defaultValue = XMFLOAT3(0.f, 0.f, 0.f);
            CopyValueFromLinkedSource(bindPin.PinId, (void*)&defaultValue);
            bindPin.Data = ParentGraph->GetNodeValue(bindPin.PinId)->GetValuePtr();
        }
        else if (bindPin.Bind.VarTypeName == "float2")
        {
            auto defaultValue = XMFLOAT2(0.f, 0.f);
            CopyValueFromLinkedSource(bindPin.PinId, (void*)&defaultValue);
            bindPin.Data = ParentGraph->GetNodeValue(bindPin.PinId)->GetValuePtr();
        }
        else if (bindPin.Bind.VarTypeName == "float")
        {
            float defaultValue = 0.f;
            CopyValueFromLinkedSource(bindPin.PinId, (void*)&defaultValue);
            bindPin.Data = ParentGraph->GetNodeValue(bindPin.PinId)->GetValuePtr();
        }
        else if (bindPin.Bind.VarTypeName == "int")
        {
            int defaultValue = 0;
            CopyValueFromLinkedSource(bindPin.PinId, (void*)&defaultValue);
            bindPin.Data = ParentGraph->GetNodeValue(bindPin.PinId)->GetValuePtr();
        }
        else if (bindPin.Bind.VarTypeName == "texture")
        {
            int defaultValue = INVALID_INDEX;
            CopyValueFromLinkedSource(bindPin.PinId, (void*)&defaultValue);
            bindPin.Data = ParentGraph->GetNodeValue(bindPin.PinId)->GetValuePtr();
        }
        else
        {
            LOG_CRITICAL("Unknown shader binding type {0}", bindPin.Bind.VarTypeName);
        }
    }

    OutputNodeValue->Value = INVALID_INDEX;
    if(isReadyToRender) 
        OutputNodeValue->Value = 1; // only set valid index when all pins are linked
}

void DrawNode::CreateShaderBindingPins(int shaderIndex)
{
    if (shaderIndex == INVALID_INDEX)
    {
        LOG_WARN("Invalid shader index{0}", shaderIndex);
        return;
    }

    Shader* shader = ShaderManager::Get()->GetShader((size_t)shaderIndex);
    if (!shader)
    {
        LOG_ERROR("Failed to load shader binding vars! Invalid shader index {0}", shaderIndex);
        return;
    }

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

            auto float4x4Value = std::make_shared<NodeValueFloat4x4>(D3DUtil::Identity4x4());
            ParentGraph->StoreNodeValue(pinId, float4x4Value);

            bindPin.Data = &float4x4Value->Value;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float4")
        {
            const Node link(NodeType::Float4, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);

            auto float4Value = std::make_shared<NodeValueFloat4>(DirectX::XMFLOAT4());
            ParentGraph->StoreNodeValue(pinId, float4Value);

            bindPin.Data = &float4Value->Value;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float3")
        {
            const Node link(NodeType::Float3, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);

            auto float3Value = std::make_shared<NodeValueFloat3>(DirectX::XMFLOAT3());
            ParentGraph->StoreNodeValue(pinId, float3Value);
                        
            bindPin.Data = &float3Value->Value;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float2")
        {
            const Node link(NodeType::Float2, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);

            auto float2Value = std::make_shared<NodeValueFloat2>(DirectX::XMFLOAT2());
            ParentGraph->StoreNodeValue(pinId, float2Value);

            bindPin.Data = &float2Value->Value;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "float")
        {
            const Node link(NodeType::Float, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);

            auto floatValue = std::make_shared<NodeValueFloat>(0.f);
            ParentGraph->StoreNodeValue(pinId, floatValue);

            bindPin.Data = &floatValue->Value;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "int")
        {
            const Node link(NodeType::Int, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);

            auto intValue = std::make_shared<NodeValueInt>(0);
            ParentGraph->StoreNodeValue(pinId, intValue);

            bindPin.Data = &intValue->Value;
            bindPin.PinId = pinId;
        }
        else if (var.VarTypeName == "texture") // texture is internaly an int that represents an index 
        {
            const Node link(NodeType::Int, NodeDirection::In);
            pinId = ParentGraph->CreateNode(link);
            ParentGraph->CreateEdge(Id, pinId, EdgeType::Internal);

            auto intValue = std::make_shared<NodeValueInt>(INVALID_INDEX);
            ParentGraph->StoreNodeValue(pinId, intValue);

            bindPin.Data = &intValue->Value;
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

void DrawNode::ClearShaderBindings()
{
    for (auto& bindPin : ShaderBindingPins)
    {
        //LOG_TRACE("Deleting shader pin nodes {0} {1} {2}", bindPin.PinId, bindPin.Bind.VarName, bindPin.Bind.VarTypeName);
        ParentGraph->EraseNode(bindPin.PinId);
    }

    ShaderBindingPins.clear();
    ShaderBindingPinNameMap.clear();
    ShaderBindingPinIdMap.clear();
}

// `from` and `to` are backwards
// `from` is the pin in this drawNode and `to` is the pin in the linked node
void DrawNode::OnShaderLinkCreate(int from, int to)
{
    //LOG_TRACE("ShaderPin link created");
    auto nodevalue = ParentGraph->GetNodeValue(to);
    int shaderIndex = *(int*)nodevalue->GetValuePtr();
    CreateShaderBindingPins(shaderIndex);
}

void DrawNode::OnShaderLinkDelete(int from, int to)
{
    //LOG_TRACE("ShaderPin link deleted");
    ClearShaderBindings();
}

void DrawNode::OnLinkedShaderUpdate(int newShaderIndex)
{
    //LOG_TRACE("Linked Shader Updated");
    auto links = ParentGraph->GetLinksConnectedTo(ShaderPin);
    for (int id : links)
        ParentGraph->EraseEdge(id); // MUST delete the link between shader and draw node to avoid DX pipeline error 
}

void DrawNode::OnLinkedTextureUpdate(int newTextureIndex)
{
    

}