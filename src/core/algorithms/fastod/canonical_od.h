#pragma once

#include "attribute_set.h"
#include "stripped_partition.h"
#include "stripped_partition_cache.h"

namespace algos::fastod {

class CanonicalOD {
private:
    AttributeSet context_;
    std::optional<SingleAttributePredicate> left_;
    size_t right_;

public:
    CanonicalOD(AttributeSet context, const SingleAttributePredicate& left, int right);
    CanonicalOD(AttributeSet context, int right);

    bool IsValid(const DataFrame& data, StrippedPartitionCache& cache) const;
    std::string ToString() const;
};

} // namespace algos::fatod
