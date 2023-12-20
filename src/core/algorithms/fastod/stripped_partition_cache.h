#pragma once

#include <memory>

#include "attribute_set.h"
#include "attribute_pair.h"
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

    bool CheckSwap(AttributeSet const& attributes, DataFrame const& data, AttributePair const& ap, bool ascending) {
        if (ContainsSpKey(attributes)) {
            return GetCachedStrippedPartition(attributes).Swap(ap.left, ap.right, ascending);
        }
        else if (ContainsRbKey(attributes)) {
            return GetCachedRangeBasedStrippedPartition(attributes).Swap(ap.left, ap.right, ascending);
        }
        
        std::optional<StrippedPartition> sp;
        std::optional<RangeBasedStrippedPartition> rb;

        auto CallProduct = [&sp, &rb](size_t attr) {
            if (sp.has_value()) {
                sp->Product(attr);
            }
            else {
                rb->Product(attr);

                if (rb->ShouldBeConvertedToStrippedPartition()) {
                    sp = std::move(rb->ToStrippedPartition());
                    rb.reset();
                    //static int c = 0;
                    //std::cout << "CONVERTED " << ++c << '\n';
                }
            }
        };

        for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
             attr = attributes.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attributes, attr);

            if (sp.has_value()) {
                if (sp_cache_.Contains(one_less)) {
                    sp = sp_cache_.Get(one_less);
                    CallProduct(attr);
                }
            }
            else {
                if (rb_cache_.Contains(one_less)) {
                    rb = rb_cache_.Get(one_less);
                    CallProduct(attr);
                }
            }
        }

        if (!sp && !rb) {
            if (!data.IsAttributesMostlyRangeBased(attributes)) {
                sp = StrippedPartition(data);
            }
            else {
                rb = RangeBasedStrippedPartition(data);
            }

            for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
                attr = attributes.find_next(attr)) {
                CallProduct(attr);
            }
        }

        if (sp.has_value()) {
            sp_cache_.Set(attributes, *sp);
            return sp->Swap(ap.left, ap.right, ascending);
        }
        else {
            rb_cache_.Set(attributes, *rb);
            return rb->Swap(ap.left, ap.right, ascending);
        }
    }

    bool CheckSplit(AttributeSet const& attributes, DataFrame const& data, short right) {
        if (ContainsSpKey(attributes)) {
            return GetCachedStrippedPartition(attributes).Split(right);
        }
        else if (ContainsRbKey(attributes)) {
            return GetCachedRangeBasedStrippedPartition(attributes).Split(right);
        }
        
        std::optional<StrippedPartition> sp;
        std::optional<RangeBasedStrippedPartition> rb;

        auto CallProduct = [&sp, &rb](size_t attr) {
            if (sp.has_value()) {
                sp->Product(attr);
            }
            else {
                rb->Product(attr);

                if (rb->ShouldBeConvertedToStrippedPartition()) {
                    sp = std::move(rb->ToStrippedPartition());
                    rb.reset();
                }
            }
        };

        for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
             attr = attributes.find_next(attr)) {
            AttributeSet one_less = deleteAttribute(attributes, attr);

            if (sp.has_value()) {
                if (sp_cache_.Contains(one_less)) {
                    sp = sp_cache_.Get(one_less);
                    CallProduct(attr);
                }
            }
            else {
                if (rb_cache_.Contains(one_less)) {
                    rb = rb_cache_.Get(one_less);
                    CallProduct(attr);
                }
            }
        }

        if (!sp && !rb) {
            if (!data.IsAttributesMostlyRangeBased(attributes)) {
                sp = StrippedPartition(data);
            }
            else {
                rb = RangeBasedStrippedPartition(data);
            }

            for (AttributeSet::size_type attr = attributes.find_first(); attr != AttributeSet::npos;
                attr = attributes.find_next(attr)) {
                CallProduct(attr);
            }
        }

        if (sp.has_value()) {
            sp_cache_.Set(attributes, *sp);
            return sp->Split(right);
        }
        else {
            rb_cache_.Set(attributes, *rb);
            return rb->Split(right);
        }
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

        if (result->ShouldBeConvertedToStrippedPartition()) {
            sp_cache_.Set(attributes, result->ToStrippedPartition());
        }
        else {
            rb_cache_.Set(attributes, *result);
        }

        return std::move(result.value());
    }
};

}  // namespace algos::fastod
