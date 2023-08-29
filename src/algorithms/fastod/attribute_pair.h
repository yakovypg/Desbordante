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
        return std::hash<size_t>()(pair.GetLeft().GetAttribute() * 32 + pair.GetRight());
    }
};

} //namespace std;
