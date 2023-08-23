#include <numeric>
#include <vector>
#include <algorithm>
#include <sstream>

#include "attribute_set.h"

namespace algos::fastod {

AttributeSet::AttributeSet() noexcept : value_(0) {}

AttributeSet::AttributeSet(int value) noexcept : value_(value) {}

AttributeSet::AttributeSet(const std::vector<int>& attributes) noexcept :
    value_(std::accumulate(attributes.cbegin(), attributes.cend(), 0, 
            [](const uint32_t& acc, const int curr){ return acc + (1 << curr); })) {}

AttributeSet::AttributeSet(const std::set<int>& set) noexcept :
    value_(std::accumulate(set.cbegin(), set.cend(), 0, 
            [](const uint32_t& acc, const int curr){ return acc + (1 << curr); })) {}

bool AttributeSet::ContainsAttribute(int attribute) const noexcept {
    return (value_ & (1 << attribute)) != 0;
}

AttributeSet AttributeSet::AddAttribute(int attribute) const noexcept{
    if(ContainsAttribute(attribute)){
        return *this;
    }
    return AttributeSet(value_ | (1 << attribute));
}

AttributeSet AttributeSet::DeleteAttribute(int attribute) const noexcept {
    if(ContainsAttribute(attribute))
        return AttributeSet(value_ ^ (1 << attribute));
    return *this;
}

AttributeSet AttributeSet::Intersect(const AttributeSet& other) const noexcept {
    return value_ & other.value_;
}

AttributeSet AttributeSet::Union(const AttributeSet& other) const noexcept {
    return value_ | other.value_;
}

AttributeSet AttributeSet::Difference(const AttributeSet& other) const noexcept {
    return value_ & ( ~0 ^ other.value_);
}

bool AttributeSet::IsEmpty() const noexcept {
    return value_ == 0;
}

std::string AttributeSet::ToString() const noexcept {
   std::stringstream ss;

   ss << "{";

   bool first = true;

   for (int attribute: *this) {
        if (first) {
            first = false;
        } else {
            ss << ",";
        }

        ss << attribute + 1;
   }

   ss << "}";

   return ss.str();
}

std::size_t AttributeSet::GetAttributeCount() const noexcept {
    size_t count = 0;
    uint32_t value = value_;
    while (value > 0) {
        ++count;
        value = value & (value - 1);
    }
    return count;
}

uint32_t AttributeSet::GetValue() const noexcept {
    return value_;
}

AttributeSetIterator AttributeSet::begin() const noexcept {
    return AttributeSetIterator(*this);
}

AttributeSetIterator AttributeSet::end() const noexcept {
    return AttributeSetIterator(*this, MAX_COLS);
}

} // namespace algos::fastod
