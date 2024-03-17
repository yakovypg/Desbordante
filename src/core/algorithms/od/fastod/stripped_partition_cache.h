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

    void CallProductWithAttribute(ComplexStrippedPartition& partition, size_t attribute) {
        partition.Product(attribute);

        if (partition.ShouldBeConvertedToStrippedPartition()) {
            partition.ToStrippedPartition();
        }
    }

    bool CallProductWithAttributesInCache(ComplexStrippedPartition& result,
                                          AttributeSet const& attribute_set) {
        bool is_product_called = false;

        for (AttributeSet::size_type attr = attribute_set.find_first();
             attr != attribute_set.size(); attr = attribute_set.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attribute_set, attr);

            if (one_less.any() && cache_.Contains(one_less)) {
                result = cache_.Get(one_less);
                CallProductWithAttribute(result, attr);
                is_product_called = true;
            }
        }

        return is_product_called;
    }

public:
    void Clear() {
        cache_.Clear();
    }

    ComplexStrippedPartition GetStrippedPartition(AttributeSet const& attribute_set,
                                                  std::shared_ptr<DataFrame> data) {
        if (cache_.Contains(attribute_set)) {
            return cache_.Get(attribute_set);
        }

        ComplexStrippedPartition result_partition;
        bool is_product_called = CallProductWithAttributesInCache(result_partition, attribute_set);

        if (!is_product_called) {
            result_partition = data->IsAttributesMostlyRangeBased(attribute_set)
                                       ? ComplexStrippedPartition::Create<true>(data)
                                       : ComplexStrippedPartition::Create<false>(data);
            for (AttributeSet::size_type attr = attribute_set.find_first();
                 attr != attribute_set.size(); attr = attribute_set.find_next(attr)) {
                CallProductWithAttribute(result_partition, attr);
            }
        }

        cache_.Set(attribute_set, result_partition);
        return result_partition;
    }
};

}  // namespace algos::fastod
