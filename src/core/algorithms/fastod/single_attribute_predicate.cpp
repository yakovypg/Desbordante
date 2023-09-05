#include "single_attribute_predicate.h"

namespace algos::fastod {

SingleAttributePredicate::SingleAttributePredicate(size_t attribute, bool ascending)
    : attribute(attribute), ascending(ascending) { }

std::string SingleAttributePredicate::ToString() const {
    return std::to_string(attribute + 1) + (ascending ? "<=" : ">=");
}

bool operator==(SingleAttributePredicate const& x, SingleAttributePredicate const& y) {
    return x.attribute == y.attribute && x.ascending == y.ascending;
}

size_t SingleAttributePredicate::hash() const {
    return attribute << (ascending ? 0 : 5);
}

} // namespace algos::fastod
