#pragma once

#include <memory>

#include "attribute_set.h"
#include "cache_with_limit.h"
#include "stripped_partition.h"
#include "range_based_stripped_partition.h"

namespace algos::fastod {

class StrippedPartitionCache {
private:
    CacheWithLimit<AttributeSet, StrippedPartition> sp_cache_{static_cast<size_t>(1e8)};
    CacheWithLimit<AttributeSet, RangeBasedStrippedPartition> rb_cache_{static_cast<size_t>(1e8)};

public:
    bool ContainsKey(AttributeSet const& attributes) const {
        return sp_cache_.Contains(attributes) || rb_cache_.Contains(attributes);
    }

    bool ContainsSpKey(AttributeSet const& attributes) const {
        return sp_cache_.Contains(attributes);
    }

    bool ContainsRbKey(AttributeSet const& attributes) const {
        return rb_cache_.Contains(attributes);
    }

    StrippedPartition const& GetCachedStrippedPartition(AttributeSet const& attributes) const {
        return sp_cache_.Get(attributes);
    }

    RangeBasedStrippedPartition const& GetCachedRangeBasedStrippedPartition(AttributeSet const& attributes) const {
        return rb_cache_.Get(attributes);
    }

    StrippedPartition CreateStrippedPartition(AttributeSet const& attributes, DataFrame const& data) {
        std::optional<StrippedPartition> result;

        auto CallProduct = [&result](size_t attr) {
            result->Product(attr);
        };

        for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
             attr = attributes.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attributes, attr);

            if (sp_cache_.Contains(one_less)) {
                result = sp_cache_.Get(one_less);
                CallProduct(attr);
            }
        }

        if (!result) {
            result = StrippedPartition(data);

            for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
                attr = attributes.find_next(attr)) {
                CallProduct(attr);
            }
        }

        sp_cache_.Set(attributes, *result);

        return std::move(result.value());
    }

    RangeBasedStrippedPartition CreateRangeBasedStrippedPartition(AttributeSet const& attributes, DataFrame const& data) {
        std::optional<RangeBasedStrippedPartition> result;

        auto CallProduct = [&result](size_t attr) {
            result->Product(attr);
        };

        for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
             attr = attributes.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attributes, attr);

            if (rb_cache_.Contains(one_less)) {
                result = rb_cache_.Get(one_less);
                CallProduct(attr);
            }
        }

        if (!result) {
            result = RangeBasedStrippedPartition(data);

            for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
                attr = attributes.find_next(attr)) {
                CallProduct(attr);
            }
        }

        rb_cache_.Set(attributes, *result);

        return std::move(result.value());
    }
};

}  // namespace algos::fastod
