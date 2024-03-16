#pragma once

#include <memory>

#include "attribute_pair.h"
#include "attribute_set.h"
#include "cache_with_limit.h"
#include "complex_stripped_partition.h"

namespace algos::fastod {

class StrippedPartitionCache {
private:
    CacheWithLimit<AttributeSet, ComplexStrippedPartition> cache_{static_cast<size_t>(1e8)};

public:
    void Clear() {
        cache_.Clear();
    }

    ComplexStrippedPartition GetStrippedPartition(AttributeSet const& attribute_set,
                                                  DataFrame const& data) {
        if (cache_.Contains(attribute_set)) {
            return cache_.Get(attribute_set);
        }

        std::optional<ComplexStrippedPartition> result;

        auto call_product = [&result](size_t attr) {
            result->Product(attr);

            if (result->ShouldBeConvertedToStrippedPartition()) {
                result->ToStrippedPartition();
            }
        };

        for (AttributeSet::size_type attr = attribute_set.find_first();
             attr != attribute_set.size(); attr = attribute_set.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attribute_set, attr);

            if (one_less.any() && cache_.Contains(one_less)) {
                result = cache_.Get(one_less);
                call_product(attr);
            }
        }

        if (!result) {
            result = data.IsAttributesMostlyRangeBased(attribute_set)
                             ? ComplexStrippedPartition::Create<true>(data)
                             : ComplexStrippedPartition::Create<false>(data);

            for (AttributeSet::size_type attr = attribute_set.find_first();
                 attr != attribute_set.size(); attr = attribute_set.find_next(attr)) {
                call_product(attr);
            }
        }

        cache_.Set(attribute_set, *result);
        return std::move(result.value());
    }
};

}  // namespace algos::fastod
