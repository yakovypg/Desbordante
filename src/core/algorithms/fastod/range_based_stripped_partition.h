#pragma once

#include <vector>

#include "data_frame.h"
#include "stripped_partition.h"

namespace algos::fastod {

class StrippedPartition;

class RangeBasedStrippedPartition {
private:
    std::vector<DataFrame::range_t> indexes_;
    std::vector<size_t> begins_;
    DataFrame const& data_;
    bool should_be_converted_to_sp_;

    static constexpr inline double SMALL_RANGES_RATIO_TO_CONVERT = 0.5;
    static constexpr inline size_t MIN_MEANINGFUL_RANGE_SIZE = static_cast<size_t>(10);

    RangeBasedStrippedPartition(DataFrame const& data,
                                std::vector<DataFrame::range_t> const& indexes,
                                std::vector<size_t> const& begins);

    std::vector<DataFrame::value_indexes_t> IntersectWithAttribute(
            algos::fastod::AttributeSet::size_type attribute, size_t group_start, size_t group_end);

public:
    explicit RangeBasedStrippedPartition(DataFrame const& data);
    RangeBasedStrippedPartition(RangeBasedStrippedPartition const& origin) = default;

    bool ShouldBeConvertedToStrippedPartition() const;

    std::string ToString() const;
    StrippedPartition ToStrippedPartition() const;

    RangeBasedStrippedPartition& operator=(RangeBasedStrippedPartition const& other);

    void Product(short attribute);
    bool Split(short right) const;
    bool Swap(short left, short right, bool ascending) const;
};

}  // namespace algos::fastod