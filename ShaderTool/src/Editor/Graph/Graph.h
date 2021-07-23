#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <stack>
#include <stddef.h>
#include <utility>
#include <vector>
#include <iostream>

#include "Edge.h"
#include "IdMap.h"


// directional graph
template<typename NodeT>
class Graph
{
public:
    Graph() : _CurrentId(0) {}

    // Element access

    NodeT& GetNode(int node_id);
    const NodeT& GetNode(int node_id) const;
    Span<const int>  GetNeighbors(int node_id) const;
    Span<const Edge> GetEdges() const;
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

    int CreateNode(const NodeT& node);        
    void EraseNode(int node_id);

    int  CreateEdge(int from, int to);    
    void EraseEdge(int edge_id);

    void SetCurrentId(int currentId) { _CurrentId = currentId; }

    void Reset()
    {
        _CurrentId = 0;
        _Nodes.clear();
        _EdgesFromNode.clear();
        _Neighbors.clear();
        _Edges.clear();
    }
    
    
    friend std::ostream& operator<<(std::ostream& out, const Graph<NodeT>& g)
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

    friend std::istream& operator>>(std::istream& in, Graph<NodeT>& g)
    {
        std::string comment;
        int numNodes;
        in >> comment >> g._CurrentId >> numNodes;

        std::string nLabel;
        int nId, nType;
        float nValue;

        for (int i = 0; i < numNodes; ++i)
        {
            in >> nLabel >> nId >> nType >> nValue;
            //LOG_TRACE("{0} {1} {2} {3}", nLabel, nId, nType, nValue);
            NodeT node(static_cast<NodeType>(nType));
            node.Value = nValue;
            g.InsertNode(nId, node);
        }

        int numEdges;
        in >> numEdges;

        std::string eLabel;
        int eId, eFrom, eTo;

        for (int i = 0; i < numEdges; ++i)
        {
            in >> eLabel >> eId >> eFrom >> eTo;
            //LOG_TRACE("{0} {1} {2} {3}", eLabel, eId, eFrom, eTo);
            g.InsertEdge(eId, eFrom, eTo);
        }

        return in;
    }
    
private:
    int  InsertNode(const int id, const NodeT& node);
    int  InsertEdge(const int id, int from, int to);

private:
    int _CurrentId;
    // These contains map to the node id
    IdMap<NodeT>            _Nodes;
    IdMap<int>              _EdgesFromNode;
    IdMap<std::vector<int>> _Neighbors;

    // This container maps to the edge id
    IdMap<Edge> _Edges;
};

template<typename NodeT>
NodeT& Graph<NodeT>::GetNode(const int id)
{
    return const_cast<NodeT&>(static_cast<const Graph*>(this)->GetNode(id));
}

template<typename NodeT>
const NodeT& Graph<NodeT>::GetNode(const int id) const
{
    const auto iter = _Nodes.find(id);
    assert(iter != _Nodes.end() && "Node not found");
    return *iter;
}

template<typename NodeT>
Span<const int> Graph<NodeT>::GetNeighbors(int node_id) const
{
    const auto iter = _Neighbors.find(node_id);
    assert(iter != _Neighbors.end());
    return *iter;
}

template<typename NodeT>
Span<const typename Edge> Graph<NodeT>::GetEdges() const
{
    return _Edges.elements();
}

template<typename NodeT>
size_t Graph<NodeT>::GetNumEdgesFromNode(const int id) const
{
    auto iter = _EdgesFromNode.find(id);
    assert(iter != _EdgesFromNode.end());
    return *iter;
}

template<typename NodeT>
int Graph<NodeT>::CreateNode(const NodeT& node)
{
    const int id = _CurrentId++;    
    return InsertNode(id, node);
}

template<typename NodeT>
int Graph<NodeT>::InsertNode(const int id, const NodeT& node)
{
    assert(!_Nodes.contains(id));
    _Nodes.insert(id, node);
    _EdgesFromNode.insert(id, 0);
    _Neighbors.insert(id, std::vector<int>());
    return id;
}

template<typename NodeT>
void Graph<NodeT>::EraseNode(const int id)
{
    // first, remove any potential dangling edges
    {
        static std::vector<int> edges_to_erase;

        for (const Edge& edge : _Edges.elements())
        {
            if (edge.contains(id))
            {
                edges_to_erase.push_back(edge.id);
            }
        }

        for (const int edge_id : edges_to_erase)
        {
            EraseEdge(edge_id);
        }

        edges_to_erase.clear();
    }

    _Nodes.erase(id);
    _EdgesFromNode.erase(id);
    _Neighbors.erase(id);
}

template<typename NodeT>
int Graph<NodeT>::CreateEdge(const int from, const int to)
{
    const int id = _CurrentId++;
    return InsertEdge(id, from, to);
}

template<typename NodeT>
int Graph<NodeT>::InsertEdge(const int id, const int from, const int to)
{
    assert(!_Edges.contains(id));
    assert(_Nodes.contains(from));
    assert(_Nodes.contains(to));
    _Edges.insert(id, Edge(id, from, to));

    // update neighbor count
    assert(_EdgesFromNode.contains(from));
    *_EdgesFromNode.find(from) += 1;
    // update neighbor list
    assert(_Neighbors.contains(from));
    _Neighbors.find(from)->push_back(to);

    return id;
}

template<typename NodeT>
void Graph<NodeT>::EraseEdge(const int edge_id)
{
    // This is a bit lazy, we find the pointer here, but we refind it when we erase the edge based
    // on id key.
    assert(_Edges.contains(edge_id));
    const Edge& edge = *_Edges.find(edge_id);

    // update neighbor count
    assert(_EdgesFromNode.contains(edge.from));
    int& edge_count = *_EdgesFromNode.find(edge.from);
    assert(edge_count > 0);
    edge_count -= 1;

    // update neighbor list
    {
        assert(_Neighbors.contains(edge.from));
        auto neighbors = _Neighbors.find(edge.from);
        auto iter = std::find(neighbors->begin(), neighbors->end(), edge.to);
        assert(iter != neighbors->end());
        neighbors->erase(iter);
    }

    _Edges.erase(edge_id);
}


template<typename NodeT, typename Visitor>
void dfs_traverse(const Graph<NodeT>& graph, const int start_node, Visitor visitor)
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
