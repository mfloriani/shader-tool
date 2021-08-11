#include "pch.h"
#include "Graph.h"

using namespace DirectX;

void Graph::Reset()
{
    _CurrentId = 0;
    _Nodes.clear();
    _EdgesFromNode.clear();
    _Neighbors.clear();
    _Edges.clear();
    
    // TODO: figure out how to handle this in a proper way
    GraphNodeValues<int>::Get().Reset();
    GraphNodeValues<float>::Get().Reset();
    GraphNodeValues<XMFLOAT2>::Get().Reset();
    GraphNodeValues<XMFLOAT3>::Get().Reset();
    GraphNodeValues<XMFLOAT4>::Get().Reset();
    GraphNodeValues<XMFLOAT4X4>::Get().Reset();
}

void Graph::StoreNodeValue(int nodeId, std::shared_ptr<GraphNodeValue> value)
{
    _NodeValueStorage[nodeId] = value;
}

std::shared_ptr<GraphNodeValue>& Graph::GetNodeValue(int nodeId)
{
    return _NodeValueStorage[nodeId];
}

Node& Graph::GetNode(const int id)
{
    return const_cast<Node&>(static_cast<const Graph*>(this)->GetNode(id));
}

const Node& Graph::GetNode(const int id) const
{
    const auto iter = _Nodes.find(id);
    assert(iter != _Nodes.end() && "Node not found");
    return *iter;
}

Span<const int> Graph::GetNeighbors(int node_id) const
{
    const auto iter = _Neighbors.find(node_id);
    assert(iter != _Neighbors.end());
    return *iter;
}

Span<const typename Edge> Graph::GetEdges() const
{
    return _Edges.elements();
}

inline Edge Graph::GetEdge(int id) const
{
    auto it = _Edges.find(id);
    if (it != _Edges.end())
        return *it;

    LOG_ERROR("Edge {0} NOT FOUND", id);
    return Edge();
}

size_t Graph::GetNumEdgesFromNode(const int id) const
{
    auto iter = _EdgesFromNode.find(id);
    assert(iter != _EdgesFromNode.end());
    return *iter;
}

int Graph::CreateNode(const Node& node)
{
    const int id = _CurrentId++;
    return InsertNode(id, node);
}

int Graph::InsertNode(const int id, const Node& node)
{
    assert(!_Nodes.contains(id));
    _Nodes.insert(id, node);
    _EdgesFromNode.insert(id, 0);
    _Neighbors.insert(id, std::vector<int>());
    return id;
}

void Graph::EraseNode(const int id)
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

int Graph::CreateEdge(int from, int to, const EdgeType type)
{
    const int id = _CurrentId++;
    return InsertEdge(id, from, to, type);
}

int Graph::InsertEdge(int id, int from, int to, const EdgeType type)
{
    assert(!_Edges.contains(id));
    assert(_Nodes.contains(from));
    assert(_Nodes.contains(to));
    _Edges.insert(id, Edge(id, from, to, type));

    //only interested in links between UiNodes
    if(type == EdgeType::External) 
        EVENT_MANAGER.Enqueue(std::make_shared<LinkCreatedEvent>(from, to));

    // update neighbor count
    assert(_EdgesFromNode.contains(from));
    *_EdgesFromNode.find(from) += 1;
    // update neighbor list
    assert(_Neighbors.contains(from));
    _Neighbors.find(from)->push_back(to);

    return id;
}

void Graph::EraseEdge(const int edge_id)
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

    EVENT_MANAGER.Enqueue(std::make_shared<LinkDeletedEvent>(edge.from, edge.to));
    //LOG_TRACE("edge deleted {0} {1} {2}", edge_id, e.from, e.to);

    _Edges.erase(edge_id);
}


