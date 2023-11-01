#pragma once

#include "stripped_partition.h"
#include "cache_with_limit.h"

#define STRIPPED_PARTITION StrippedPartition

namespace algos::fastod {

class StrippedPartitionCache {
private:
    CacheWithLimit<AttributeSet, STRIPPED_PARTITION> cache_{static_cast<size_t>(1e8)};

public:
    STRIPPED_PARTITION GetStrippedPartition(AttributeSet const& attribute_set,
                                           DataFrame const& data) {
        if (cache_.Contains(attribute_set)) {
            return cache_.Get(attribute_set);
        }

        std::optional<STRIPPED_PARTITION> result;

        auto callProduct = [&result](size_t attr) {
            result->Product(attr);
        };

        for (AttributeSet::size_type attr = attribute_set.find_first(); attr != AttributeSet::npos;
             attr = attribute_set.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attribute_set, attr);

            if (cache_.Contains(one_less)) {
                result = cache_.Get(one_less);
                callProduct(attr);
            }
        }

        if (!result) {
            result = STRIPPED_PARTITION(data);

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