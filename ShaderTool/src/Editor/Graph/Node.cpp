#include "pch.h"
#include "Node.h"

HlslNodeType* HlslNodeType::_Instance = nullptr;

HlslType HlslNodeType::GetHlslType(const std::string& typeName)
{
    auto it = _HlslTypeNameMap.find(typeName);
    if (it == _HlslTypeNameMap.end())
    {
        LOG_ERROR("Invalid type name {0}", typeName);
        return { NodeType::None, "INVALID TYPE", 0 };
    }
    return _HlslTypes[it->second];
}

HlslType HlslNodeType::GetHlslType(NodeType nodeType)
{
    auto it = _HlslTypeNodeMap.find(nodeType);
    if (it == _HlslTypeNodeMap.end())
    {
        LOG_ERROR("Invalid node type {0}", nodeType);
        return { NodeType::None, "INVALID TYPE", 0 };
    }
    return _HlslTypes[it->second];
}

UINT HlslNodeType::GetNum32BitValues(std::string typeName)
{
    return GetHlslType(typeName).Num32BitValues;
}

UINT HlslNodeType::GetNum32BitValues(NodeType nodeType)
{
    return GetHlslType(nodeType).Num32BitValues;
}