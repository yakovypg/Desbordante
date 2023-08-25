#pragma once

#include <vector>

#include "data_frame.h"
#include "schema_value.h"
#include "single_attribute_predicate.h"
#include "cache_with_limit.h"
#include "attribute_set.h"

namespace algos::fastod {

class StrippedPartition {
private:
    std::vector<int> indexes_;
    std::vector<int> begins_;
    const DataFrame& data_;
    static CacheWithLimit<size_t, StrippedPartition> cache_;
    
public:
    StrippedPartition();
    explicit StrippedPartition(const DataFrame& data) noexcept;
    StrippedPartition(const StrippedPartition& origin) noexcept;

    void Product(int attribute) noexcept;

    bool Split(int right) noexcept;
    bool Swap(const SingleAttributePredicate& left, int right) noexcept;

    std::string ToString() const noexcept;
    StrippedPartition DeepClone() const noexcept;
    static StrippedPartition GetStrippedPartition(size_t attribute_set, const DataFrame& data) noexcept;

    long SplitRemoveCount(int right) noexcept;
    long SwapRemoveCount(const SingleAttributePredicate& left, int right) noexcept;

    StrippedPartition& operator=(const StrippedPartition& other);
};

} // namespace algos::fastod
