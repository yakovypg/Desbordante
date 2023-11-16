#pragma once

#include <vector>
#include <iostream>

#include "data_frame.h"
#include "stripped_partition.h"
#include "partition.h"

namespace algos::fastod {

class StrippedPartition;

class RangeBasedStrippedPartition : public Partition {
private:
    std::vector<DataFrame::range_t> indexes_;
    std::vector<size_t> begins_;
    const DataFrame& data_;

    RangeBasedStrippedPartition(const DataFrame& data, std::vector<DataFrame::range_t> const& indexes, std::vector<size_t> const& begins);

    std::vector<DataFrame::value_indexes_t> IntersectWithAttribute(
        algos::fastod::AttributeSet::size_type attribute,
        size_t group_start,
        size_t group_end);
    
public:
    explicit RangeBasedStrippedPartition(const DataFrame& data);
    RangeBasedStrippedPartition(const RangeBasedStrippedPartition& origin) = default;

    StrippedPartition ToStrippedPartition() const;
    RangeBasedStrippedPartition& operator=(const RangeBasedStrippedPartition& other);

    std::string ToString() const override;
    std::shared_ptr<Partition> Copy() const override;

    void Product(short attribute) override;
    bool Split(short right) override;
    bool Swap(short left, short right, bool ascending) override;
};

} // namespace algos::fastod