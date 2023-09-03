#pragma once

#include "stripped_partition.h"
#include "cache_with_limit.h"

namespace algos::fastod {


template <bool multithread>
class StrippedPartitionCache {
private:
    CacheWithLimit<size_t, StrippedPartition<multithread>, multithread> cache_{static_cast<size_t>(1e8)};

public:
    StrippedPartition<multithread> GetStrippedPartition(size_t attribute_set,
                                                        const DataFrame& data) noexcept {
        if (cache_.Contains(attribute_set)) {
            return cache_.Get(attribute_set);
        }

        std::optional<StrippedPartition<multithread>> result;

        auto callProduct = [&result, &data](size_t attr) {
            if (data.GetColTypeId(attr) == +model::TypeId::kInt)
                result->template Product<int>(attr);
            else if (data.GetColTypeId(attr) == +model::TypeId::kDouble)
                result->template Product<double>(attr);
            else
                result->template Product<std::string>(attr);
        };

        for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
            size_t one_less = deleteAttribute(attribute_set, *attr);

            if (cache_.Contains(one_less)) {
                result = cache_.Get(one_less);
                callProduct(*attr);
            }
        }

        if (!result) {
            result = StrippedPartition<multithread>(data);

            for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
                callProduct(*attr);
            }
        }

        cache_.Set(attribute_set, *result);

        return std::move(result.value());
    }
};

}  // namespace algos::fastod
