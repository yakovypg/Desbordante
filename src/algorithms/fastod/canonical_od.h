#pragma once

#include <optional>

#include "data_frame.h"
#include "single_attribute_predicate.h"
#include "attribute_set.h"
#include "stripped_partition.h"

namespace algos::fastod {

class CanonicalOD {
private:
    size_t context_;
    std::optional<SingleAttributePredicate> const left_;
    int const right_;

public:
    CanonicalOD(size_t context, const SingleAttributePredicate& left, int right) noexcept;
    CanonicalOD(size_t context, int right) noexcept;

    template <typename TL, typename TR>
    bool IsValid(const DataFrame& data, double error_rate_threshold) const noexcept {
        // important
        StrippedPartition sp = StrippedPartition::GetStrippedPartition(context_, data);

        if (error_rate_threshold == -1) {
            if (!left_)
                return !sp.Split<TR>(right_);

            // important
            return !sp.Swap<TL, TR>(*left_, right_);
        }

        long violation_count;

        if (!left_)
            violation_count = sp.SplitRemoveCount<TR>(right_);
        else
            violation_count = sp.SwapRemoveCount<TL, TR>(left_.value(), right_);

        double error_rate = (double)violation_count / data.GetTupleCount();

        return error_rate < error_rate_threshold;
    }
    std::string ToString() const noexcept;

    friend bool operator==(CanonicalOD const& x, CanonicalOD const& y);
    friend bool operator<(CanonicalOD const& x, CanonicalOD const& y);
};

} // namespace algos::fatod