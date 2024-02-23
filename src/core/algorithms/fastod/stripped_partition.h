#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "attribute_set.h"
#include "range_based_stripped_partition.h"
#include "single_attribute_predicate.h"

namespace algos::fastod {

class RangeBasedStrippedPartition;

class StrippedPartition {
private:
    std::vector<size_t> indexes_;
    std::vector<size_t> begins_;
    DataFrame const& data_;

    StrippedPartition(DataFrame const& data, std::vector<size_t> const& indexes,
                      std::vector<size_t> const& begins);
    friend class RangeBasedStrippedPartition;

public:
    explicit StrippedPartition(DataFrame const& data);
    StrippedPartition(StrippedPartition const& origin) = default;

    StrippedPartition& operator=(StrippedPartition const& other);

    std::string ToString() const;

    void Product(short attribute);
    bool Split(short right) const;
    bool Swap(short left, short right, bool ascending) const;
};

}  // namespace algos::fastod
