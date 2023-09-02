#include <sstream>

#include "canonical_od.h"
#include "single_attribute_predicate.h"
#include "timer.h"

namespace algos::fastod {

template <bool multithread>
CanonicalOD<multithread>::CanonicalOD(size_t context, const SingleAttributePredicate& left, int right) noexcept :
    context_(context), left_(left), right_(right) {}

template <bool multithread>
CanonicalOD<multithread>::CanonicalOD(size_t context, int right) noexcept : context_(context), right_(right) {}

template <bool multithread>
std::string CanonicalOD<multithread>::ToString() const noexcept {
    std::stringstream ss;

    ss << ASToString(context_) << " : ";

    if (left_)
        ss << left_->ToString() << " ~ ";
    else
        ss << "[] -> ";

    ss << right_ + 1 << "<=";

    return ss.str();
}

template class CanonicalOD<false>;
template class CanonicalOD<true>;

} // namespace algos::fastod
