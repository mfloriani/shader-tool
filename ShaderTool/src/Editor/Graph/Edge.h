#pragma once

#include <iostream>

enum class EdgeType
{
    Internal, // for internal links of a same uinode
    External  // for input/output links between uinodes 
};

struct Edge
{
    int id;
    int from, to;
    EdgeType type;

    Edge() = default;
    Edge(const int id, const int f, const int t, EdgeType type) : id(id), from(f), to(t), type(type) {}

    inline int  opposite(const int n) const { return n == from ? to : from; }
    inline bool contains(const int n) const { return n == from || n == to; }

    friend std::ostream& operator<<(std::ostream& out, const Edge& e)
    {
        out << e.id
            << " "
            << e.from
            << " "
            << e.to
            << " "
            << (int)e.type
            << " "
            << magic_enum::enum_name(e.type);
        return out;
    }

    friend std::istream& operator>>(std::istream& in, Edge& e)
    {
        std::string typeName;
        int type;
        in >> e.id >> e.from >> e.to >> type >> typeName;
        e.type = (EdgeType)type;
        return in;
    }
};