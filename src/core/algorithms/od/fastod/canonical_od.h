#pragma once

#include <memory>

#include "attribute_pair.h"
#include "stripped_partition.h"
#include "stripped_partition_cache.h"

namespace algos::fastod {

template <bool ascending>
class CanonicalOD {
private:
    AttributeSet context_;
    AttributePair ap_;

public:
    CanonicalOD(AttributeSet&& context, AttributeSet::size_type left,
                AttributeSet::size_type right);

    bool IsValid(std::shared_ptr<DataFrame> data, StrippedPartitionCache& cache) const;
    std::string ToString() const;
};

using AscCanonicalOD = CanonicalOD<true>;
using DescCanonicalOD = CanonicalOD<false>;

class SimpleCanonicalOD {
private:
    AttributeSet context_;
    AttributeSet::size_type right_;

public:
    SimpleCanonicalOD(AttributeSet const& context, AttributeSet::size_type right);

    bool IsValid(std::shared_ptr<DataFrame> data, StrippedPartitionCache& cache) const;
    std::string ToString() const;
};

}  // namespace algos::fastod
