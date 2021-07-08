#pragma once

#include <vector>
#include <iterator>
#include <algorithm>
#include <utility>

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

    const_iterator begin() const { return _Elements.begin(); }
    const_iterator end() const { return _Elements.end(); }

    // Element access

    Span<const ElementType> elements() const { return _Elements; }
    const std::vector<int>& ids() const { return _SortedIds; }

    // Capacity

    bool   empty() const { return _SortedIds.empty(); }
    size_t size() const { return _SortedIds.size(); }

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
    std::vector<ElementType> _Elements;
    std::vector<int>         _SortedIds;
};

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int          id,
    const ElementType& element)
{
    auto lower_bound = std::lower_bound(_SortedIds.begin(), _SortedIds.end(), id);

    if (lower_bound != _SortedIds.end() && id == *lower_bound)
    {
        return std::make_pair(std::next(_Elements.begin(), std::distance(_SortedIds.begin(), lower_bound)), false);
    }

    auto insert_element_at = std::next(_Elements.begin(), std::distance(_SortedIds.begin(), lower_bound));

    _SortedIds.insert(lower_bound, id);
    return std::make_pair(_Elements.insert(insert_element_at, element), true);
}

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int     id,
    ElementType&& element)
{
    auto lower_bound = std::lower_bound(_SortedIds.begin(), _SortedIds.end(), id);

    if (lower_bound != _SortedIds.end() && id == *lower_bound)
    {
        return std::make_pair(std::next(_Elements.begin(), std::distance(_SortedIds.begin(), lower_bound)), false);
    }

    auto insert_element_at = std::next(_Elements.begin(), std::distance(_SortedIds.begin(), lower_bound));

    _SortedIds.insert(lower_bound, id);
    return std::make_pair(_Elements.insert(insert_element_at, std::move(element)), true);
}

template<typename ElementType>
size_t IdMap<ElementType>::erase(const int id)
{
    auto lower_bound = std::lower_bound(_SortedIds.begin(), _SortedIds.end(), id);

    if (lower_bound == _SortedIds.end() || id != *lower_bound)
    {
        return 0ull;
    }

    auto erase_element_at =
        std::next(_Elements.begin(), std::distance(_SortedIds.begin(), lower_bound));

    _SortedIds.erase(lower_bound);
    _Elements.erase(erase_element_at);

    return 1ull;
}

template<typename ElementType>
void IdMap<ElementType>::clear()
{
    _Elements.clear();
    _SortedIds.clear();
}

template<typename ElementType>
typename IdMap<ElementType>::iterator IdMap<ElementType>::find(const int id)
{
    const auto lower_bound = std::lower_bound(_SortedIds.cbegin(), _SortedIds.cend(), id);
    return (lower_bound == _SortedIds.cend() || *lower_bound != id)
        ? _Elements.end()
        : std::next(_Elements.begin(), std::distance(_SortedIds.cbegin(), lower_bound));
}

template<typename ElementType>
typename IdMap<ElementType>::const_iterator IdMap<ElementType>::find(const int id) const
{
    const auto lower_bound = std::lower_bound(_SortedIds.cbegin(), _SortedIds.cend(), id);
    return (lower_bound == _SortedIds.cend() || *lower_bound != id)
        ? _Elements.cend()
        : std::next(_Elements.cbegin(), std::distance(_SortedIds.cbegin(), lower_bound));
}

template<typename ElementType>
bool IdMap<ElementType>::contains(const int id) const
{
    const auto lower_bound = std::lower_bound(_SortedIds.cbegin(), _SortedIds.cend(), id);

    if (lower_bound == _SortedIds.cend())
    {
        return false;
    }

    return *lower_bound == id;
}