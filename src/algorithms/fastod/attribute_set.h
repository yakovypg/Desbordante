#pragma once

#include <set>
#include <vector>
#include <string>
#include <iostream>

namespace algos::fastod {

constexpr int MAX_COLS = 8 * sizeof(uint32_t);

class AttributeSetIterator;

class AttributeSet {
private:
    uint32_t value_;

public:
    AttributeSet() noexcept;
    AttributeSet(int attribute) noexcept;
    explicit AttributeSet(const std::vector<int>& attributes) noexcept;
    explicit AttributeSet(const std::set<int>& set) noexcept;

    bool ContainsAttribute(int attribute) const noexcept;
    AttributeSet AddAttribute(int attribute) const noexcept;
    AttributeSet DeleteAttribute(int attribute) const noexcept;

    AttributeSet Intersect(const AttributeSet& other) const noexcept;
    AttributeSet Union(const AttributeSet& other) const noexcept;
    AttributeSet Difference(const AttributeSet& other) const noexcept;

    bool IsEmpty() const noexcept;
    std::string ToString() const noexcept;

    std::size_t GetAttributeCount() const noexcept;
    uint32_t GetValue() const noexcept;

    AttributeSetIterator begin() const noexcept;
    AttributeSetIterator end() const noexcept;

    friend bool operator==(AttributeSet const& x, AttributeSet const& y) {
        return x.value_ == y.value_;
    }
    friend bool operator<(const AttributeSet& x, const AttributeSet& y) {
        return x.value_ < y.value_;
    }

    AttributeSet& operator=(const AttributeSet& other) = default;
};

struct AttributeSetIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = int;
    using pointer           = int*;
    using reference         = int&;

    AttributeSetIterator(const AttributeSetIterator&) = default;
    AttributeSetIterator(const AttributeSet& value, int poss = 0) : value_(value), pos(poss) {
        while (pos < MAX_COLS && !value_.ContainsAttribute(pos))
            ++pos;
    }

    reference operator*() { return pos; }
    pointer operator->() { return &pos; }
    AttributeSetIterator& operator++() {
        if (pos < MAX_COLS) {
            ++pos;
            while (pos < MAX_COLS && !value_.ContainsAttribute(pos))
                ++pos;
        }
        return *this;
    }  
    AttributeSetIterator operator++(int) {
        AttributeSetIterator tmp = *this;
        ++(*this);
        return tmp; 
    }
    friend bool operator== (const AttributeSetIterator& a, const AttributeSetIterator& b) { return a.value_ == b.value_ && a.pos == b.pos; };
    friend bool operator!= (const AttributeSetIterator& a, const AttributeSetIterator& b) { return !(a == b); };

private:
    AttributeSet value_;
    int pos = 0;
};

} // namespace algos::fastod 

template <>
struct std::hash<algos::fastod::AttributeSet>
{
    size_t operator()(const algos::fastod::AttributeSet& attr_set) const {
        return attr_set.GetValue();
    }
};
