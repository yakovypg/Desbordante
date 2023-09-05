#pragma once

#include <cstddef>
#include <string>

namespace algos::fastod {

class ASIterator;

size_t attributeSet(const std::initializer_list<size_t>&& attributes);
bool containsAttribute(size_t value, size_t attribute);
size_t addAttribute(size_t value, size_t attribute);
size_t deleteAttribute(size_t value, size_t attribute);
size_t intersect(size_t value1, size_t value2);
size_t difference(size_t value1, size_t value2);
bool isEmptyAS(size_t value);
std::string ASToString(size_t value);
std::size_t getAttributeCount(size_t value);

ASIterator attrsBegin(size_t value);
ASIterator attrsEnd(size_t value);

struct ASIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = size_t;
    using pointer           = size_t*;
    using reference         = size_t&;
    static size_t MAX_COLS;

    ASIterator(const ASIterator&) = default;
    ASIterator(size_t value, size_t pos = 0);

    reference operator*();
    pointer operator->();
    ASIterator& operator++();
    ASIterator operator++(int);
    friend bool operator==(const ASIterator& a, const ASIterator& b);
    friend bool operator!=(const ASIterator& a, const ASIterator& b);

private:
    size_t value_;
    size_t pos_;
};

} // namespace algos::fastod
