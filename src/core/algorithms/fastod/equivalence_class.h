#pragma once

#include <vector>
#include <string>

#include "range.h"
#include "data_frame.h"

namespace algos::fastod {

class DataFrame;

class EquivalenceClass {
public:
    EquivalenceClass() noexcept = default;
    EquivalenceClass(EquivalenceClass const& other) = default;
    EquivalenceClass(AttributeSet::size_type attribute) noexcept;
    EquivalenceClass(std::vector<Range> indexes, AttributeSet::size_type attribute);
    
    size_t Size() const noexcept;
    std::vector<Range> const& GetIndexes() const;
    AttributeSet::size_type GetAttribute() const;

    std::string ToString() const;
    std::string ToStringWithSort() const;

    int GetRepresentative(DataFrame const& data) const;
    int GetRepresentative(DataFrame const& data, algos::fastod::AttributeSet::size_type attribute) const;
    
    std::vector<int> GetValues(DataFrame const& data) const;
    std::vector<std::pair<int, size_t>> GetIndexedValues(DataFrame const& data) const;
    
    std::vector<std::pair<int, size_t>> GetIndexedValues(DataFrame const& data,
        algos::fastod::AttributeSet::size_type attribute) const;
    
    static EquivalenceClass Intersect(EquivalenceClass const& lhs, EquivalenceClass const& rhs);
    
    static std::vector<EquivalenceClass> ExtractClasses(std::vector<int> const& data,
        algos::fastod::AttributeSet::size_type attribute);

private:
    std::vector<Range> indexes_;
    size_t size_;
    AttributeSet::size_type attribute_;
};

} // namespace algos::fastod
