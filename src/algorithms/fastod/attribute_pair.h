#pragma once

#include "single_attribute_predicate.h"

namespace algos::fastod {

class AttributePair {
private:
    const std::pair<SingleAttributePredicate, size_t> pair_;

public:
    AttributePair(const SingleAttributePredicate& left, size_t right) noexcept;

    const SingleAttributePredicate& GetLeft() const noexcept;
    size_t GetRight() const noexcept;

    std::string ToString() const;

    friend bool operator==(const AttributePair& x, const AttributePair& y);
};

} // namespace algos::fastod 

namespace std {

template <>
struct hash<algos::fastod::AttributePair> {
    std::size_t operator()(const algos::fastod::AttributePair& pair) const {
        auto left_hash = std::hash<size_t>()(pair.GetLeft().GetHashCode());
        auto right_hash = std::hash<size_t>()(pair.GetRight());
        return left_hash ^ right_hash;
    }
};

} //namespace std;
