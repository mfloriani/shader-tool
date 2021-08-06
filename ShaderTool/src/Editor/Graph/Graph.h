#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <stack>
#include <stddef.h>
#include <utility>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "Node.h"
#include "Edge.h"
#include "IdMap.h"
#include "Events/EventManager.h"
#include "Events/Event.h"

// directional graph
class Graph
{
public:
    Graph() : _CurrentId(0) {}

    // Element access

    Node& GetNode(int node_id);
    const Node& GetNode(int node_id) const;
    Span<const int>  GetNeighbors(int node_id) const;
    Span<const Edge> GetEdges() const;
    Edge GetEdge(int id) const;
    const std::vector<int>& GetNodes() const { return _Nodes.ids(); }
    
    const std::vector<int>& GetEdgesFromNodeIds() const { return _EdgesFromNode.ids(); }
    Span<const int> GetEdgesFromNode() const { return _EdgesFromNode.elements(); }
    
    const std::vector<int>& GetAllNeighborIds() const { return _Neighbors.ids(); }
    Span<const std::vector<int>> GetAllNeighbors() const { return _Neighbors.elements(); }

    int GetCurrentId() const { return _CurrentId; }
    size_t GetNodesCount() const { return _Nodes.size(); }

    // Capacity
    
    size_t GetNumEdgesFromNode(int node_id) const;
    size_t GetEdgesCount() const { return _Edges.size(); }

    // Modifiers

    int CreateNode(const Node& node);        
    void EraseNode(int node_id);

    int  CreateEdge(int from, int to, const EdgeType type);
    void EraseEdge(int edge_id);

    void SetCurrentId(int currentId) { _CurrentId = currentId; }

    void Reset();
    
    

    
    friend std::ostream& operator<<(std::ostream& out, const Graph& g)
    {
        out << g._CurrentId << "\n";
        out << g._Nodes.size() << "\n";
        
        auto& ids = g._Nodes.ids();
        int i = 0;
        for (auto it = g._Nodes.begin(); it != g._Nodes.end(); ++it, ++i)
            out << "n " << ids[i] << " " << *it << "\n";

        out << g._Edges.size() << "\n";
        auto edges = g._Edges.elements();
        for (auto it = edges.begin(); it != edges.end(); ++it)
            out << "e " << *it << "\n";
        
        return out;
    }

    friend std::istream& operator>>(std::istream& in, Graph& g)
    {
        std::string comment;
        int numNodes;
        in >> comment >> g._CurrentId >> numNodes;

        std::string nLabel;
        int nId;
        Node node;
        for (int i = 0; i < numNodes; ++i)
        {
            
            in >> nLabel >> nId >> node;
            g.InsertNode(nId, node);
            //LOG_TRACE("{0} {1} {2} {3}", nLabel, nId, nType, nValue);
        }

        int numEdges;
        in >> numEdges;

        std::string eLabel;
        Edge edge;
        for (int i = 0; i < numEdges; ++i)
        {
            in >> eLabel >> edge;
            g.InsertEdge(edge.id, edge.from, edge.to, edge.type);
            //LOG_TRACE("{0} {1} {2} {3}", eLabel, eId, eFrom, eTo);
        }

        return in;
    }
    
private:
    int  InsertNode(const int id, const Node& node);
    int  InsertEdge(const int id, int from, int to, const EdgeType type);

private:
    int _CurrentId;
    
    IdMap<Node>             _Nodes;         // These contains map to the node id
    IdMap<int>              _EdgesFromNode;
    IdMap<std::vector<int>> _Neighbors;    
    IdMap<Edge>             _Edges;        // This container maps to the edge id
};

template<typename Visitor>
void dfs_traverse(const Graph& graph, const int start_node, Visitor visitor)
{
    std::stack<int> stack;
    stack.push(start_node);

    while (!stack.empty())
    {
        const int current_node = stack.top();
        stack.pop();

        visitor(current_node);

        for (const int neighbor : graph.GetNeighbors(current_node))
        {
            stack.push(neighbor);
        }
    }
}

template<class T>
class GraphNodeValues
{
public:
    static GraphNodeValues<T>& Get()
    {
        static GraphNodeValues<T> instance;
        return instance;
    }
    
    void StoreNodeValuePtr(int id, std::shared_ptr<NodeValue<T>> value)
    {
        _NodeValues[id] = value;
    }

    std::shared_ptr<NodeValue<T>>& GetNodeValuePtr(int id)
    {
        return _NodeValues[id];
    }

    void Reset()
    {
        _NodeValues.clear();
    }

private:
    GraphNodeValues() = default;

private:
    std::unordered_map<NodeId, std::shared_ptr<NodeValue<T>>> _NodeValues;
};

