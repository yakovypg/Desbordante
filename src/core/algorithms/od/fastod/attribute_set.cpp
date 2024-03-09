#include "attribute_set.h"

#include <numeric>
#include <sstream>

namespace algos::fastod {

std::string ASToString(AttributeSet const& value) {
    std::stringstream ss;
    ss << "{";
    bool first = true;
    for (AttributeSet::size_type i = value.find_first(); i != value.size();
         i = value.find_next(i)) {
        if (first)
            first = false;
        else
            ss << ",";
        ss << i + 1;
    }
    ss << "}";
    return ss.str();
}

}  // namespace algos::fastod
