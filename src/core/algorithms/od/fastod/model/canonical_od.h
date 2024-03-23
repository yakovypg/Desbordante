#pragma once

#include <memory>

#include "algorithms/od/fastod/storage/partition_cache.h"
#include "attribute_pair.h"

namespace algos::fastod {

template <bool ascending>
class CanonicalOD {
private:
    AttributeSet context_;
    AttributePair ap_;

public:
    CanonicalOD(AttributeSet&& context, model::ColumnIndex left, model::ColumnIndex right);

    bool IsValid(std::shared_ptr<DataFrame> data, PartitionCache& cache) const;
    std::string ToString() const;
};

using AscCanonicalOD = CanonicalOD<true>;
using DescCanonicalOD = CanonicalOD<false>;

class SimpleCanonicalOD {
private:
    AttributeSet context_;
    model::ColumnIndex right_;

public:
    SimpleCanonicalOD(AttributeSet const& context, model::ColumnIndex right);

    bool IsValid(std::shared_ptr<DataFrame> data, PartitionCache& cache) const;
    std::string ToString() const;
};

}  // namespace algos::fastod
