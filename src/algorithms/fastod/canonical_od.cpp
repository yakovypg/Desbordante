#include <sstream>

#include "canonical_od.h"
#include "operator_type.h"
#include "single_attribute_predicate.h"
#include "timer.h"

using namespace algos::fastod;

CanonicalOD::CanonicalOD(size_t context, const SingleAttributePredicate& left, int right) noexcept :
    context_(context), left_(std::move(left)), right_(right) {}


CanonicalOD::CanonicalOD(size_t context, int right) noexcept : context_(context), left_({}), right_(right) {}

std::string CanonicalOD::ToString() const noexcept {
    std::stringstream ss;

    ss << ASToString(context_) << " : ";

    if (left_)
        ss << left_->ToString() << " ~ ";
    else
        ss << "[] -> ";

    ss << right_ + 1 << "<=";

    return ss.str();
}

namespace algos::fastod {

bool operator==(CanonicalOD const& x, CanonicalOD const& y) {
    return x.context_ == y.context_
        && x.left_ == y.left_
        && x.right_ == y.right_;
}

// TODO: Check whether x and y should be swapped
bool operator<(CanonicalOD const& x, CanonicalOD const& y) {
    const int attribute_count_difference = getAttributeCount(x.context_) - getAttributeCount(y.context_);
    if (attribute_count_difference != 0) {
        return attribute_count_difference < 0;
    }

    const int context_value_difference = x.context_ - y.context_;
    if (context_value_difference != 0) {
        return context_value_difference < 0;
    }

    const int right_difference = x.right_ - y.right_;
    if (right_difference != 0) {
        return right_difference < 0;
    }

    if (!x.left_.has_value()) {
        return false;
    }

    const int left_difference = x.left_.value().GetAttribute() - y.left_.value().GetAttribute();
    if (left_difference != 0) {
        return left_difference < 0;
    }

    if (x.left_.value().GetOperator() == y.left_.value().GetOperator()) {
        return false;
    }

    if (x.left_.value().GetOperator().GetType() == OperatorType::LessOrEqual) {
        return true;    //TODO: Make sure -1 in Java code corrensponds to true
    }

    return false;
}

} // namespace algos::fastod
