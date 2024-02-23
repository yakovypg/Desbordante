#include "canonical_od.h"

#include <sstream>

#include "timer.h"

namespace algos::fastod {

template <bool ascending>
CanonicalOD<ascending>::CanonicalOD(AttributeSet&& context, AttributeSet::size_type left,
                                    AttributeSet::size_type right)
    : context_(std::move(context)), ap_(left, right) {}

template <bool ascending>
bool CanonicalOD<ascending>::IsValid(DataFrame const& data, StrippedPartitionCache& cache) const {
    return !(cache.GetStrippedPartition(context_, data).Swap<ascending>(ap_.left, ap_.right));
}

template <bool ascending>
std::string CanonicalOD<ascending>::ToString() const {
    std::stringstream ss;
    ss << ASToString(context_) << " : " << ap_.left + 1 << (ascending ? "<=" : ">=") << " ~ "
       << ap_.right + 1 << "<=";
    return ss.str();
}

SimpleCanonicalOD::SimpleCanonicalOD(AttributeSet const& context, AttributeSet::size_type right)
    : context_(context), right_(right) {}

bool SimpleCanonicalOD::IsValid(DataFrame const& data, StrippedPartitionCache& cache) const {
    return !(cache.GetStrippedPartition(context_, data).Split(right_));
}

std::string SimpleCanonicalOD::ToString() const {
    std::stringstream ss;
    ss << ASToString(context_) << " : [] -> " << right_ + 1 << "<=";
    return ss.str();
}

template class CanonicalOD<true>;
template class CanonicalOD<false>;

}  // namespace algos::fastod
