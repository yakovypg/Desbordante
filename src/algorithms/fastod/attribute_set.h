#pragma once

#include <set>
#include <vector>
#include <string>
#include <iostream>

namespace algos::fastod {

constexpr int MAX_COLS = 8 * sizeof(uint32_t);

class ASIterator;

size_t attributeSet(const std::initializer_list<int>&& attributes) noexcept;
bool containsAttribute(size_t value, size_t attribute) noexcept;
size_t addAttribute(size_t value, int attribute) noexcept;
size_t deleteAttribute(size_t value, size_t attribute) noexcept;
size_t intersect(size_t value1, size_t value2) noexcept;
size_t difference(size_t value1, size_t value2) noexcept;
bool isEmptyAS(size_t value) noexcept;
std::string ASToString(size_t value) noexcept;
std::size_t getAttributeCount(size_t value) noexcept;

ASIterator attrsBegin(size_t value) noexcept;
ASIterator attrsEnd(size_t value) noexcept;

struct ASIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = int;
    using pointer           = int*;
    using reference         = int&;

    ASIterator(const ASIterator&) = default;
    ASIterator(size_t value, int pos = 0);

    reference operator*();
    pointer operator->();
    ASIterator& operator++();
    ASIterator operator++(int);
    friend bool operator==(const ASIterator& a, const ASIterator& b);
    friend bool operator!=(const ASIterator& a, const ASIterator& b);

private:
    size_t value_;
    int pos_;
};

} // namespace algos::fastod
