#pragma once

#include <memory>

#include "partition_dispatcher.h"
#include "attribute_set.h"
#include "cache_with_limit.h"
#include "partition.h"
#include "stripped_partition.h"
#include "range_based_stripped_partition.h"

namespace algos::fastod {

class StrippedPartitionCache {
private:
    CacheWithLimit<AttributeSet, std::shared_ptr<Partition>> cache_{static_cast<size_t>(1e8)};
    PartitionDispatcher partition_dispatcher_;

public:
    void ConfigurePartitionDispatcher(DataFrame const& data) {
        partition_dispatcher_.GenerateStartPartitions(std::move(data));
    }

    std::shared_ptr<Partition> GetStrippedPartition(AttributeSet const& attribute_set,
                                                    DataFrame const& data) {
        if (cache_.Contains(attribute_set)) {
            return cache_.Get(attribute_set);
        }

        std::optional<std::shared_ptr<Partition>> result;

        auto callProduct = [&result](size_t attr) {
            (*result)->Product(attr);
        };

        for (AttributeSet::size_type attr = attribute_set.find_first(); attr != AttributeSet::npos;
             attr = attribute_set.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attribute_set, attr);

            if (cache_.Contains(one_less)) {
                result = cache_.Get(one_less)->Copy();
                callProduct(attr);
            }
        }

        if (!result) {
            result = partition_dispatcher_.GetPrefferedPartition(attribute_set, data);

            for (AttributeSet::size_type attr = attribute_set.find_first(); attr != AttributeSet::npos;
                attr = attribute_set.find_next(attr)) {
                callProduct(attr);
            }
        }

        cache_.Set(attribute_set, *result);

        return std::move(result.value());
    }
};

}  // namespace algos::fastod
