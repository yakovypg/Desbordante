#pragma once

#include <vector>
#include <string>

#include "range.h"
#include "data_frame.h"

namespace algos::fastod {

class EquivalenceClass {
public:
    EquivalenceClass() noexcept = default;
    EquivalenceClass(AttributeSet::size_type attribute) noexcept;
    EquivalenceClass(std::vector<Range> indexes, AttributeSet::size_type attribute);
    
    size_t Size() const noexcept;
    std::vector<Range> const& GetIndexes() const;

    std::string ToString() const;

    int GetRepresentative(DataFrame const& data) const;
    int GetRepresentative(DataFrame const& data, algos::fastod::AttributeSet::size_type attribute) const;
    
    std::vector<int> GetValues(DataFrame const& data) const;
    std::vector<std::pair<int, size_t>> GetIndexedValues(DataFrame const& data) const;
    
    std::vector<std::pair<int, size_t>> GetIndexedValues(DataFrame const& data,
        algos::fastod::AttributeSet::size_type attribute) const;

private:
    std::vector<Range> indexes_;
    size_t size_;
    AttributeSet::size_type attribute_;
};

} // namespace algos::fastod
