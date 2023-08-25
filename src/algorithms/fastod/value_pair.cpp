#include <sstream>
#include <utility>

#include "schema_value.h"
#include "value_pair.h"

using namespace algos::fastod;

ValuePair::ValuePair(SchemaValue const& first, SchemaValue const& second) noexcept : pair_(std::make_pair(first, second)) {}

const SchemaValue& ValuePair::GetFirst() const noexcept {
    return pair_.first;
}

const SchemaValue& ValuePair::GetSecond() const noexcept {
    return pair_.second;
}

SchemaValue& ValuePair::GetFirst() noexcept {
    return pair_.first;
}

SchemaValue& ValuePair::GetSecond() noexcept {
    return pair_.second;
}

std::string ValuePair::ToString() const noexcept {
    std::stringstream ss;

    ss << "DataAndIndex{first=" << GetFirst().ToString() << ", second=" << GetSecond().ToString() << "}";

    return ss.str();
}

namespace algos::fastod {

bool operator<(const ValuePair& x, const ValuePair& y) {
    return x.GetFirst() < y.GetFirst();
}

bool operator>(const ValuePair& x, const ValuePair& y) {
    return y.GetFirst() < x.GetFirst();
}

} // namespace algos::fastod
