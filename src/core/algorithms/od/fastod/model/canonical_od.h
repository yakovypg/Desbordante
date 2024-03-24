#pragma once

#include <memory>

#include "algorithms/od/fastod/hashing/hashing.h"
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

    friend struct std::hash<CanonicalOD<ascending>>;
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

    friend struct std::hash<SimpleCanonicalOD>;
};

}  // namespace algos::fastod

namespace std {

template <bool ascending>
struct hash<algos::fastod::CanonicalOD<ascending>> {
    size_t operator()(algos::fastod::CanonicalOD<ascending> const& od) const noexcept {
        const size_t context_hash = hash<algos::fastod::AttributeSet>{}(od.context_);
        const size_t ap_hash = hash<algos::fastod::AttributePair>{}(od.ap_);

        return algos::fastod::hashing::CombineHashes(context_hash, ap_hash);
    }
};

template <>
struct hash<algos::fastod::SimpleCanonicalOD> {
    size_t operator()(algos::fastod::SimpleCanonicalOD const& od) const noexcept {
        const size_t context_hash = hash<algos::fastod::AttributeSet>{}(od.context_);
        const size_t right_hash = hash<model::ColumnIndex>{}(od.right_);

        return algos::fastod::hashing::CombineHashes(context_hash, right_hash);
    }
};

}  // namespace std
