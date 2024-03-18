#include "attribute_set.h"

#include <functional>
#include <numeric>
#include <sstream>

namespace algos::fastod {

void AttributeSet::iterate(std::function<void(size_type)> callback) const {
    for (size_type attr = find_first(); attr != size(); attr = find_next(attr)) {
        callback(attr);
    }
}

std::string ASToString(AttributeSet const& value) {
    std::stringstream ss;
    ss << "{";

    bool first = true;

    value.iterate([&ss, &first](AttributeSet::size_type i) {
        if (first)
            first = false;
        else
            ss << ",";

        ss << i + 1;
    });

    ss << "}";

    return ss.str();
}

}  // namespace algos::fastod
