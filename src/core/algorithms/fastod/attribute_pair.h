#pragma once

#include "attribute_set.h"

namespace algos::fastod {

struct AttributePair {
public:
    AttributeSet::size_type left;
    AttributeSet::size_type right;

    AttributePair(AttributeSet::size_type, AttributeSet::size_type);
    std::string ToString() const;
};
bool operator==(const AttributePair& x, const AttributePair& y);

} // namespace algos::fastod 

namespace std {

template <>
struct hash<algos::fastod::AttributePair> {
    std::size_t operator()(const algos::fastod::AttributePair& pair) const {
        return std::hash<size_t>()((pair.left << 10) + pair.right);
    }
};

} //namespace std;
