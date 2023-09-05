#include "attribute_pair.h"
#include <string>

namespace algos::fastod {

AttributePair::AttributePair(SingleAttributePredicate&& left, size_t right) :
    left(left), right(right) {}

std::string AttributePair::ToString() const {
    return "{ " + left.ToString() + ", " + std::to_string(right + 1) + " }";
}

bool operator==(const AttributePair& x, const AttributePair& y) {
    return x.left == y.left && x.right == y.right;
}

} // namespace algos::fastod
