#pragma once

#include <optional>

#include "data_frame.h"
#include "single_attribute_predicate.h"
#include "attribute_set.h"

namespace algos::fastod {

class CanonicalOD {
private:
    size_t context_;
    std::optional<SingleAttributePredicate> const left_;
    int const right_;
    static int split_check_count_;
    static int swap_check_count_;

public:
    CanonicalOD(size_t context, const SingleAttributePredicate& left, int right) noexcept;
    CanonicalOD(size_t context, int right) noexcept;

    bool IsValid(const DataFrame& data, double error_rate_threshold) const noexcept;
    std::string ToString() const noexcept;

    friend bool operator==(CanonicalOD const& x, CanonicalOD const& y);
    friend bool operator<(CanonicalOD const& x, CanonicalOD const& y);
};

} // namespace algos::fatod
