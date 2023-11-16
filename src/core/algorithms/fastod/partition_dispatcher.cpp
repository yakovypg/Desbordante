#include "partition_dispatcher.h"
#include "data_frame.h"

namespace algos::fastod {

void PartitionDispatcher::GenerateStartPartitions(DataFrame const& data) {
    GenerateStrippedPartitions(data, START_STRIPPED_PARTITIONS_COUNT);
    GenerateRangeBasedPartitions(data, START_RANGE_BASED_PARTITIONS_COUNT);
}

std::shared_ptr<Partition> PartitionDispatcher::GetPrefferedPartition(AttributeSet const& attributes, DataFrame const& data) {
    AttributeSet remaining_attrs = intersect(data.GetAttributesWithRanges(), attributes);

    AttributeSet::size_type attrs_count = attributes.count();
    AttributeSet::size_type remaining_attrs_count = remaining_attrs.count();

    const double accept_range_based_partition_factor = 0.5;

    if ((double)remaining_attrs_count / attrs_count >= accept_range_based_partition_factor) {
        if (used_range_based_partitions_ == range_based_partitions_.size()) {
            ClearRangeBasedPartitions();
            GenerateRangeBasedPartitions(std::move(data));
        }
        //std::cout << "ACCEPT " << (double)remaining_attrs_count / attrs_count;
        return range_based_partitions_[used_range_based_partitions_++];
    }
    
    if (used_stripped_partitions_ == stripped_partitions_.size()) {
        ClearStrippedPartitions();
        GenerateStrippedPartitions(std::move(data));
    }

    return stripped_partitions_[used_stripped_partitions_++];
}

void PartitionDispatcher::ClearStrippedPartitions() {
    stripped_partitions_.clear();
    used_stripped_partitions_ = 0;
}

void PartitionDispatcher::ClearRangeBasedPartitions() {
    range_based_partitions_.clear();
    used_range_based_partitions_ = 0;
}

void PartitionDispatcher::GenerateStrippedPartitions(DataFrame const& data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        stripped_partitions_.push_back(std::shared_ptr<Partition>(new StrippedPartition(data)));
    }
}

void PartitionDispatcher::GenerateRangeBasedPartitions(DataFrame const& data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        range_based_partitions_.push_back(std::shared_ptr<Partition>(new RangeBasedStrippedPartition(data)));
    }
}

} // namespace algos::fastod
