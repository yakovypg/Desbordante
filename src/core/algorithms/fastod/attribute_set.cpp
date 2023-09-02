#include <numeric>
#include <sstream>

#include "attribute_set.h"

namespace algos::fastod {

size_t ASIterator::MAX_COLS = 32;

size_t attributeSet(const std::initializer_list<size_t>&& attributes) noexcept {
    return std::accumulate(attributes.begin(), attributes.end(), 0, 
        [](size_t acc, size_t curr){ return acc + (1 << curr); });
}

bool containsAttribute(size_t value, size_t attribute) noexcept {
    return (value & (1 << attribute)) != 0;
}

size_t addAttribute(size_t value, size_t attribute) noexcept {
    if(containsAttribute(value, attribute)){
        return value;
    }
    return value | (1 << attribute);
}

size_t deleteAttribute(size_t value, size_t attribute) noexcept {
    if(containsAttribute(value, attribute))
        return value ^ (1 << attribute);
    return value;
}

size_t intersect(size_t value1, size_t value2) noexcept {
    return value1 & value2;
}

size_t difference(size_t value1, size_t value2) noexcept {
    return value1 & ( ~0 ^ value2);
}

bool isEmptyAS(size_t value) noexcept {
    return value == 0;
}

std::string ASToString(size_t value) noexcept {
    std::stringstream ss;
   ss << "{";
   bool first = true;
   for (ASIterator it = attrsBegin(value); it != attrsEnd(value); ++it) {
        if (first)
            first = false;
        else
            ss << ",";
        ss << *it + 1;
   }
   ss << "}";
   return ss.str();
}

std::size_t getAttributeCount(size_t value) noexcept {
    size_t count = 0;
    while (value > 0) {
        ++count;
        value = value & (value - 1);
    }
    return count;
}

ASIterator::ASIterator(size_t value, size_t pos) : value_(value), pos_(pos) {
    while (pos_ < MAX_COLS && !containsAttribute(value_, pos_))
        ++pos_;
}

ASIterator attrsBegin(size_t value) noexcept {
    return ASIterator(value);
}

ASIterator attrsEnd(size_t value) noexcept {
    return ASIterator(value, ASIterator::MAX_COLS);
}

ASIterator::reference ASIterator::operator*() { return pos_; }
ASIterator::pointer ASIterator::operator->() { return &pos_; }
ASIterator& ASIterator::operator++() {
    if (pos_ < MAX_COLS)
        while ((++pos_) < MAX_COLS && (value_ & (1 << pos_)) == 0);
    return *this;
}  
ASIterator ASIterator::operator++(int) {
    ASIterator tmp = *this;
    ++(*this);
    return tmp; 
}

bool operator==(const ASIterator& a, const ASIterator& b) { 
    return a.pos_ == b.pos_ && a.value_ == b.value_; 
};

bool operator!=(const ASIterator& a, const ASIterator& b) {
    return !(a == b);
};

} // namespace algos::fastod
