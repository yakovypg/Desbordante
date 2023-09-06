#include <sstream>

#include "canonical_od.h"
#include "single_attribute_predicate.h"
#include "timer.h"

namespace algos::fastod {

CanonicalOD::CanonicalOD(AttributeSet context, const SingleAttributePredicate& left, int right) :
    context_(std::move(context)), left_(left), right_(right) {}
CanonicalOD::CanonicalOD(AttributeSet context, int right) :
    context_(std::move(context)), right_(right) {}

bool CanonicalOD::IsValid(const DataFrame& data, StrippedPartitionCache& cache) const {
    StrippedPartition sp = cache.GetStrippedPartition(context_, data);
    if (!left_)
        return !(sp.Split(right_));

    return !(sp.Swap(*left_, right_));
}

std::string CanonicalOD::ToString() const {
    std::stringstream ss;

    ss << ASToString(context_) << " : ";

    if (left_)
        ss << left_->ToString() << " ~ ";
    else
        ss << "[] -> ";

    ss << right_ + 1 << "<=";

    return ss.str();
}

} // namespace algos::fastod
