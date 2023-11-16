#pragma once

#include <memory>
#include <vector>

#include "attribute_set.h"
#include "partition.h"
#include "stripped_partition.h"
#include "range_based_stripped_partition.h"

namespace algos::fastod {

class PartitionDispatcher {
public:
    PartitionDispatcher() = default;

    void GenerateStartPartitions(DataFrame const& data);
    std::shared_ptr<Partition> GetPrefferedPartition(AttributeSet const& attributes, DataFrame const& data);

private:
    static constexpr inline size_t PARTITION_SET_SIZE = 1000;
    static constexpr inline size_t START_STRIPPED_PARTITIONS_COUNT = 100 * 1000;
    static constexpr inline size_t START_RANGE_BASED_PARTITIONS_COUNT = 100 * 1000;
    
    size_t used_stripped_partitions_;
    size_t used_range_based_partitions_;

    std::vector<std::shared_ptr<Partition>> stripped_partitions_;
    std::vector<std::shared_ptr<Partition>> range_based_partitions_;

    void ClearStrippedPartitions();
    void ClearRangeBasedPartitions();

    void GenerateStrippedPartitions(DataFrame const& data, size_t count = PARTITION_SET_SIZE);
    void GenerateRangeBasedPartitions(DataFrame const& data, size_t count = PARTITION_SET_SIZE);
};

} // namespave algos::fastod