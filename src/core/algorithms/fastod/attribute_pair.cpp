#include "attribute_pair.h"
#include <string>

namespace algos::fastod {

AttributePair::AttributePair(AttributeSet::size_type left, AttributeSet::size_type right) :
    left(left), right(right) {}

std::string AttributePair::ToString() const {
    return "{ " + std::to_string(left + 1) + ", " + std::to_string(right + 1) + " }";
}

bool operator==(const AttributePair& x, const AttributePair& y) {
    return x.left == y.left && x.right == y.right;
}

} // namespace algos::fastod
