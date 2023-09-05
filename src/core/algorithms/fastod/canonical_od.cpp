#include <sstream>

#include "canonical_od.h"
#include "single_attribute_predicate.h"
#include "timer.h"

namespace algos::fastod {

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
