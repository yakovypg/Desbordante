#pragma once

#include "stripped_partition.h"
#include "cache_with_limit.h"

namespace algos::fastod {

class StrippedPartitionCache {
private:
    CacheWithLimit<size_t, StrippedPartition> cache_{static_cast<size_t>(1e8)};

public:
    StrippedPartition GetStrippedPartition(size_t attribute_set,
                                                        const DataFrame& data) noexcept {
        if (cache_.Contains(attribute_set)) {
            return cache_.Get(attribute_set);
        }

        std::optional<StrippedPartition> result;

        auto callProduct = [&result](size_t attr) {
            result->Product(attr);
        };

        for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
            size_t one_less = deleteAttribute(attribute_set, *attr);

            if (cache_.Contains(one_less)) {
                result = cache_.Get(one_less);
                callProduct(*attr);
            }
        }

        if (!result) {
            result = StrippedPartition(data);

            for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
                callProduct(*attr);
            }
        }

        cache_.Set(attribute_set, *result);

        return std::move(result.value());
    }
};

}  // namespace algos::fastod
