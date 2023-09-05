#pragma once

#include "stripped_partition.h"
#include "stripped_partition_cache.h"

namespace algos::fastod {

class CanonicalOD {
private:
    size_t context_;
    std::optional<SingleAttributePredicate> const left_;
    size_t const right_;

public:
    CanonicalOD(size_t context, const SingleAttributePredicate& left, int right) noexcept;
    CanonicalOD(size_t context, int right) noexcept;

    bool IsValid(const DataFrame& data, double error_rate_threshold,
                 StrippedPartitionCache& cache) const noexcept {
        // important
        StrippedPartition sp = cache.GetStrippedPartition(context_, data);

        if (error_rate_threshold == -1) {
            if (!left_)
                return !(sp.Split(right_));

            // important
            return !(sp.Swap(*left_, right_));
        }

        long violation_count;

        if (!left_)
            violation_count = sp.SplitRemoveCount(right_);
        else
            violation_count = sp.SwapRemoveCount(left_.value(), right_);

        double error_rate = (double)violation_count / data.GetTupleCount();

        return error_rate < error_rate_threshold;
    }
    std::string ToString() const noexcept;
};

} // namespace algos::fatod
