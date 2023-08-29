#include "single_attribute_predicate.h"

namespace algos::fastod {

SingleAttributePredicate::SingleAttributePredicate(size_t attribute, bool ascending) noexcept
    : attribute_(attribute), ascending_(ascending) { }

size_t SingleAttributePredicate::GetAttribute() const noexcept {
    return attribute_;
}

bool SingleAttributePredicate::GetAsc() const noexcept {
    return ascending_;
}

std::string SingleAttributePredicate::ToString() const {
    return std::to_string(attribute_ + 1) + (ascending_ ? "<=" : ">=");
}

bool operator==(SingleAttributePredicate const& x, SingleAttributePredicate const& y) {
    return x.attribute_ == y.attribute_ && x.ascending_ == y.ascending_;
}

} // namespace algos::fastod

