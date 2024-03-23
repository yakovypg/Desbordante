#include "attribute_set.h"

#include <functional>
#include <numeric>
#include <sstream>

namespace algos::fastod {

std::string AttributeSet::ToString() const {
    std::stringstream result;
    result << "{";

    bool first = true;

    Iterate([&result, &first](AttributeSet::size_type i) {
        if (first)
            first = false;
        else
            result << ",";

        result << i + 1;
    });

    result << "}";

    return result.str();
}

void AttributeSet::Iterate(std::function<void(size_type)> callback) const {
    for (size_type attr = FindFirst(); attr != Size(); attr = FindNext(attr)) {
        callback(attr);
    }
}

}  // namespace algos::fastod
