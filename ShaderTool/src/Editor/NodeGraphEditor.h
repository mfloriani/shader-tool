#pragma once

#include "Defines.h"
#include "Node.h"
#include "Graph.h"
#include "GameTimer.h"

#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"

#include <vector>

#define VK_S 0x53
#define VK_L 0x4C
#define VK_N 0x4E

class NodeGraphEditor
{
public:
    NodeGraphEditor() : _RootNodeId(-1) {}

    void Init();
    void Quit();
    void Render();

private:
    void Reset();
    void Save();    
    void Load();
    
private:
    Graph<Node>         _Graph;
    std::vector<UiNode> _UINodes;
    int                 _RootNodeId;
};

static NodeGraphEditor NodeGraphEd;