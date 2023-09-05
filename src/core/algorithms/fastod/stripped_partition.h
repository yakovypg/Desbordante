#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#include "single_attribute_predicate.h"
#include "attribute_set.h"

namespace algos::fastod {

class StrippedPartition {
private:
    std::vector<size_t> indexes_;
    std::vector<size_t> begins_;
    const DataFrame& data_;
    
public:
    explicit StrippedPartition(const DataFrame& data);
    StrippedPartition(const StrippedPartition& origin) = default;

    std::string ToString() const;
    StrippedPartition& operator=(const StrippedPartition& other);

    void Product(size_t attribute);
    bool Split(size_t right);
    bool Swap(const SingleAttributePredicate& left, size_t right);
    long SplitRemoveCount(size_t right);
    long SwapRemoveCount(const SingleAttributePredicate& left, size_t right);
};

} // namespace algos::fastod
