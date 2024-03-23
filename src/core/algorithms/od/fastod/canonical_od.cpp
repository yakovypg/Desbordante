#include "canonical_od.h"

#include <memory>
#include <sstream>

#include "timer.h"

namespace algos::fastod {

template <bool ascending>
CanonicalOD<ascending>::CanonicalOD(AttributeSet&& context, AttributeSet::size_type left,
                                    AttributeSet::size_type right)
    : context_(std::move(context)), ap_(left, right) {}

template <bool ascending>
bool CanonicalOD<ascending>::IsValid(std::shared_ptr<DataFrame> data,
                                     StrippedPartitionCache& cache) const {
    return !(cache.GetStrippedPartition(context_, data).Swap<ascending>(ap_.left, ap_.right));
}

template <bool ascending>
std::string CanonicalOD<ascending>::ToString() const {
    std::stringstream result;

    result << context_.ToString() << " : " << ap_.left + 1 << (ascending ? "<=" : ">=") << " ~ "
           << ap_.right + 1 << "<=";

    return result.str();
}

SimpleCanonicalOD::SimpleCanonicalOD(AttributeSet const& context, AttributeSet::size_type right)
    : context_(context), right_(right) {}

bool SimpleCanonicalOD::IsValid(std::shared_ptr<DataFrame> data,
                                StrippedPartitionCache& cache) const {
    return !(cache.GetStrippedPartition(context_, data).Split(right_));
}

std::string SimpleCanonicalOD::ToString() const {
    std::stringstream result;
    result << context_.ToString() << " : [] -> " << right_ + 1 << "<=";

    return result.str();
}

template class CanonicalOD<true>;
template class CanonicalOD<false>;

}  // namespace algos::fastod
