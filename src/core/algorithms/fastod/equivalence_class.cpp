#include <string>
#include <numeric>
#include <sstream>

#include "equivalence_class.h"

namespace algos::fastod {

EquivalenceClass::EquivalenceClass(AttributeSet::size_type attribute) noexcept
    : indexes_({}), size_(0), attribute_(attribute) {}

EquivalenceClass::EquivalenceClass(std::vector<Range> indexes, AttributeSet::size_type attribute) {  
    attribute_ = attribute;
    indexes_ = std::move(indexes);
    
    size_ = std::accumulate(indexes_.begin(), indexes_.end(), 0, [](size_t sum, Range const& range) {
        return sum + range.Size();
    });
}

size_t EquivalenceClass::Size() const noexcept {
    return size_;
}

std::vector<Range> const& EquivalenceClass::GetIndexes() const {
    return indexes_;
}

std::string EquivalenceClass::ToString() const {
    std::stringstream ss;
    size_t curr_index = 0;

    ss << "{ ";
    
    for (Range const& range : indexes_) {
        ss << range.ToString();

        if (curr_index++ < indexes_.size() - 1) {
            ss << ", ";
        }
    }

    ss << " }";

    return ss.str();
}

int EquivalenceClass::GetRepresentative(DataFrame const& data) const {
    return GetRepresentative(data, attribute_);
}

int EquivalenceClass::GetRepresentative(DataFrame const& data, algos::fastod::AttributeSet::size_type attribute) const {
    return size_ > 0
        ? data.GetValue(indexes_[0].GetStart(), attribute)
        : 0;
}

std::vector<int> EquivalenceClass::GetValues(DataFrame const& data) const {
    std::vector<int> values(size_);  
    size_t curr_index = 0;

    for (Range const& range : indexes_) {
        for (size_t index = range.GetStart(); index <= range.GetEnd(); ++index) {
            values[curr_index++] = data.GetValue(index, attribute_);
        }
    }

    return values;
}

std::vector<std::pair<int, size_t>> EquivalenceClass::GetIndexedValues(DataFrame const& data) const {
    return GetIndexedValues(data, attribute_);
}

std::vector<std::pair<int, size_t>> EquivalenceClass::GetIndexedValues(DataFrame const& data,
    algos::fastod::AttributeSet::size_type attribute) const {
    
    std::vector<std::pair<int, size_t>> values(size_); 
    size_t curr_index = 0;

    for (Range const& range : indexes_) {
        for (size_t index = range.GetStart(); index <= range.GetEnd(); ++index) {
            values[curr_index++] = std::make_pair(data.GetValue(index, attribute), index);
        }
    }

    return values;
}

} // namespace algos::fastod
