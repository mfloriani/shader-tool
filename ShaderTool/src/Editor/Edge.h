#pragma once

#include <iostream>

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