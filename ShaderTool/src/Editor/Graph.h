#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <stack>
#include <stddef.h>
#include <utility>
#include <vector>
#include <iostream>

template<typename ElementType>
struct Span
{
    using iterator = ElementType*;

    template<typename Container>
    Span(Container& c) : begin_(c.data()), end_(begin_ + c.size()), size_(c.size())
    {
    }

    iterator begin() const { return begin_; }
    iterator end() const { return end_; }

    size_t size() const { return size_; }

private:
    iterator begin_;
    iterator end_;
    size_t size_;
};

template<typename ElementType>
class IdMap
{
public:
    using iterator = typename std::vector<ElementType>::iterator;
    using const_iterator = typename std::vector<ElementType>::const_iterator;

    // Iterators

    const_iterator begin() const { return elements_.begin(); }
    const_iterator end() const { return elements_.end(); }

    // Element access

    Span<const ElementType> elements() const { return elements_; }
    const std::vector<int>& ids() const { return sorted_ids_; }

    // Capacity

    bool   empty() const { return sorted_ids_.empty(); }
    size_t size() const { return sorted_ids_.size(); }

    // Modifiers

    std::pair<iterator, bool> insert(int id, const ElementType& element);
    std::pair<iterator, bool> insert(int id, ElementType&& element);
    size_t                    erase(int id);
    void                      clear();

    // Lookup

    iterator       find(int id);
    const_iterator find(int id) const;
    bool           contains(int id) const;

private:
    std::vector<ElementType> elements_;
    std::vector<int>         sorted_ids_;
};

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int          id,
    const ElementType& element)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound != sorted_ids_.end() && id == *lower_bound)
    {
        return std::make_pair(std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound)), false);
    }

    auto insert_element_at = std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.insert(lower_bound, id);
    return std::make_pair(elements_.insert(insert_element_at, element), true);
}

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int     id,
    ElementType&& element)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound != sorted_ids_.end() && id == *lower_bound)
    {
        return std::make_pair(std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound)), false);
    }

    auto insert_element_at = std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.insert(lower_bound, id);
    return std::make_pair(elements_.insert(insert_element_at, std::move(element)), true);
}

template<typename ElementType>
size_t IdMap<ElementType>::erase(const int id)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound == sorted_ids_.end() || id != *lower_bound)
    {
        return 0ull;
    }

    auto erase_element_at =
        std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.erase(lower_bound);
    elements_.erase(erase_element_at);

    return 1ull;
}

template<typename ElementType>
void IdMap<ElementType>::clear()
{
    elements_.clear();
    sorted_ids_.clear();
}

template<typename ElementType>
typename IdMap<ElementType>::iterator IdMap<ElementType>::find(const int id)
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);
    return (lower_bound == sorted_ids_.cend() || *lower_bound != id)
        ? elements_.end()
        : std::next(elements_.begin(), std::distance(sorted_ids_.cbegin(), lower_bound));
}

template<typename ElementType>
typename IdMap<ElementType>::const_iterator IdMap<ElementType>::find(const int id) const
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);
    return (lower_bound == sorted_ids_.cend() || *lower_bound != id)
        ? elements_.cend()
        : std::next(elements_.cbegin(), std::distance(sorted_ids_.cbegin(), lower_bound));
}

template<typename ElementType>
bool IdMap<ElementType>::contains(const int id) const
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);

    if (lower_bound == sorted_ids_.cend())
    {
        return false;
    }

    return *lower_bound == id;
}

// a very simple directional graph
template<typename NodeT>
class Graph
{
public:
    Graph() : current_id_(0), nodes_(), edges_from_node_(), node_neighbors_(), edges_() {}

    struct Edge
    {
        int id;
        int from, to;

        Edge() = default;
        Edge(const int id, const int f, const int t) : id(id), from(f), to(t) {}

        inline int  opposite(const int n) const { return n == from ? to : from; }
        inline bool contains(const int n) const { return n == from || n == to; }

        friend std::ostream& operator<<(std::ostream& out, const Edge& e)
        {
            out << e.id
                << " "
                << e.from
                << " "
                << e.to;
            return out;
        }
    };

    // Element access

    NodeT& node(int node_id);
    const NodeT& node(int node_id) const;
    Span<const int>  neighbors(int node_id) const;
    Span<const Edge> edges() const;
    const std::vector<int>& nodes() const { return nodes_.ids(); }
    
    const std::vector<int>& edgesFromNodeIds() const { return edges_from_node_.ids(); }
    Span<const int> edgesFromNode() const { return edges_from_node_.elements(); }
    
    const std::vector<int>& allNeighborIds() const { return node_neighbors_.ids(); }
    Span<const std::vector<int>> allNeighbors() const { return node_neighbors_.elements(); }

    int GetCurrentId() const { return current_id_; }
    

    size_t GetNodesCount() const { return nodes_.size(); }

    // Capacity

    size_t num_edges_from_node(int node_id) const;

    // Modifiers
    int create_node(const NodeT& node);        
    void erase_node(int node_id);

    int  create_edge(int from, int to);    
    void erase_edge(int edge_id);

    void SetCurrentId(int currentId) { current_id_ = currentId; }

    void reset()
    {
        int current_id_ = 0;
        nodes_.clear();
        edges_from_node_.clear();
        node_neighbors_.clear();
        edges_.clear();
    }
    
    friend std::ostream& operator<<(std::ostream& out, const Graph<NodeT>& g)
    {
        out << g.current_id_ << "\n";
        out << g.nodes_.size() << "\n";
        
        auto& ids = g.nodes_.ids();
        int i = 0;
        for (auto it = g.nodes_.begin(); it != g.nodes_.end(); ++it, ++i)
            out << "n " << ids[i] << " " << *it << "\n";

        out << g.edges_.size() << "\n";
        auto edges = g.edges_.elements();
        for (auto it = edges.begin(); it != edges.end(); ++it)
            out << "e " << *it << "\n";
        
        return out;
    }

    friend std::istream& operator>>(std::istream& in, Graph<NodeT>& g)
    {
        std::string comment;
        int numNodes;
        in >> comment >> g.current_id_ >> numNodes;

        std::string nLabel;
        int nId, nType;
        float nValue;

        for (int i = 0; i < numNodes; ++i)
        {
            in >> nLabel >> nId >> nType >> nValue;
            //LOG_TRACE("{0} {1} {2} {3}", nLabel, nId, nType, nValue);
            NodeT node(static_cast<NodeType>(nType));
            node.value = nValue;
            g.insert_node(nId, node);
        }

        int numEdges;
        in >> numEdges;

        std::string eLabel;
        int eId, eFrom, eTo;

        for (int i = 0; i < numEdges; ++i)
        {
            in >> eLabel >> eId >> eFrom >> eTo;
            //LOG_TRACE("{0} {1} {2} {3}", eLabel, eId, eFrom, eTo);
            g.insert_edge(eId, eFrom, eTo);
        }

        return in;
    }
    
private:
    int  insert_node(const int id, const NodeT& node);
    int  insert_edge(const int id, int from, int to);

private:
    int current_id_;
    // These contains map to the node id
    IdMap<NodeT>            nodes_;
    IdMap<int>              edges_from_node_;
    IdMap<std::vector<int>> node_neighbors_;

    // This container maps to the edge id
    IdMap<Edge> edges_;
};

template<typename NodeT>
NodeT& Graph<NodeT>::node(const int id)
{
    return const_cast<NodeT&>(static_cast<const Graph*>(this)->node(id));
}

template<typename NodeT>
const NodeT& Graph<NodeT>::node(const int id) const
{
    const auto iter = nodes_.find(id);
    assert(iter != nodes_.end());
    return *iter;
}

template<typename NodeT>
Span<const int> Graph<NodeT>::neighbors(int node_id) const
{
    const auto iter = node_neighbors_.find(node_id);
    assert(iter != node_neighbors_.end());
    return *iter;
}

template<typename NodeT>
Span<const typename Graph<NodeT>::Edge> Graph<NodeT>::edges() const
{
    return edges_.elements();
}

template<typename NodeT>
size_t Graph<NodeT>::num_edges_from_node(const int id) const
{
    auto iter = edges_from_node_.find(id);
    assert(iter != edges_from_node_.end());
    return *iter;
}

template<typename NodeT>
int Graph<NodeT>::create_node(const NodeT& node)
{
    const int id = current_id_++;    
    return insert_node(id, node);
}

template<typename NodeT>
int Graph<NodeT>::insert_node(const int id, const NodeT& node)
{
    assert(!nodes_.contains(id));
    nodes_.insert(id, node);
    edges_from_node_.insert(id, 0);
    node_neighbors_.insert(id, std::vector<int>());
    return id;
}

template<typename NodeT>
void Graph<NodeT>::erase_node(const int id)
{

    // first, remove any potential dangling edges
    {
        static std::vector<int> edges_to_erase;

        for (const Edge& edge : edges_.elements())
        {
            if (edge.contains(id))
            {
                edges_to_erase.push_back(edge.id);
            }
        }

        for (const int edge_id : edges_to_erase)
        {
            erase_edge(edge_id);
        }

        edges_to_erase.clear();
    }

    nodes_.erase(id);
    edges_from_node_.erase(id);
    node_neighbors_.erase(id);
}

template<typename NodeT>
int Graph<NodeT>::create_edge(const int from, const int to)
{
    const int id = current_id_++;
    return insert_edge(id, from, to);
}

template<typename NodeT>
int Graph<NodeT>::insert_edge(const int id, const int from, const int to)
{
    assert(!edges_.contains(id));
    assert(nodes_.contains(from));
    assert(nodes_.contains(to));
    edges_.insert(id, Edge(id, from, to));

    // update neighbor count
    assert(edges_from_node_.contains(from));
    *edges_from_node_.find(from) += 1;
    // update neighbor list
    assert(node_neighbors_.contains(from));
    node_neighbors_.find(from)->push_back(to);

    return id;
}

template<typename NodeT>
void Graph<NodeT>::erase_edge(const int edge_id)
{
    // This is a bit lazy, we find the pointer here, but we refind it when we erase the edge based
    // on id key.
    assert(edges_.contains(edge_id));
    const Edge& edge = *edges_.find(edge_id);

    // update neighbor count
    assert(edges_from_node_.contains(edge.from));
    int& edge_count = *edges_from_node_.find(edge.from);
    assert(edge_count > 0);
    edge_count -= 1;

    // update neighbor list
    {
        assert(node_neighbors_.contains(edge.from));
        auto neighbors = node_neighbors_.find(edge.from);
        auto iter = std::find(neighbors->begin(), neighbors->end(), edge.to);
        assert(iter != neighbors->end());
        neighbors->erase(iter);
    }

    edges_.erase(edge_id);
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

        for (const int neighbor : graph.neighbors(current_node))
        {
            stack.push(neighbor);
        }
    }
}
