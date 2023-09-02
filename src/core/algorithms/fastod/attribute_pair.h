#pragma once

#include "single_attribute_predicate.h"

namespace algos::fastod {

struct AttributePair {
public:
    SingleAttributePredicate left;
    size_t right;
    AttributePair(SingleAttributePredicate&& left, size_t right) noexcept;
    std::string ToString() const;
};
bool operator==(const AttributePair& x, const AttributePair& y);

} // namespace algos::fastod 

namespace std {

template <>
struct hash<algos::fastod::AttributePair> {
    std::size_t operator()(const algos::fastod::AttributePair& pair) const {
        return std::hash<size_t>()((pair.left.hash() << 10) + pair.right);
    }
};

} //namespace std;
