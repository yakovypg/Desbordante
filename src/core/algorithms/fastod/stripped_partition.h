#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#include "range_based_stripped_partition.h"
#include "single_attribute_predicate.h"
#include "attribute_set.h"

namespace algos::fastod {

class RangeBasedStrippedPartition;

class StrippedPartition {
private:
    std::vector<size_t> indexes_;
    std::vector<size_t> begins_;
    const DataFrame& data_;

    StrippedPartition(const DataFrame& data, std::vector<size_t> const& indexes, std::vector<size_t> const& begins);
    friend class RangeBasedStrippedPartition;

public:
    explicit StrippedPartition(const DataFrame& data);
    StrippedPartition(const StrippedPartition& origin) = default;

    StrippedPartition& operator=(const StrippedPartition& other);

    std::string ToString() const;
    
    void Product(short attribute);
    bool Split(short right) const;
    bool Swap(short left, short right, bool ascending) const;
};

} // namespace algos::fastod
