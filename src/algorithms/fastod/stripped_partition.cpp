#include <cstdint>
#include <sstream>
#include "stripped_partition.h"
#include "cache_with_limit.h"
// #include "timer.h"

namespace algos::fastod {

template <bool multithread>
StrippedPartition<multithread>::StrippedPartition(const DataFrame& data) : data_(std::move(data)) {
    indexes_.reserve(data.GetTupleCount());
    for (size_t i = 0; i < data.GetTupleCount(); i++) {
        indexes_.push_back(i);
    }

    if (data.GetTupleCount() != 0) {
        begins_.push_back(0);
    }

    begins_.push_back(data.GetTupleCount());
}

template <bool multithread>
std::string StrippedPartition<multithread>::ToString() const noexcept {
    std::stringstream ss;
    std::string indexes_string;

    for (size_t i = 0; i < indexes_.size(); i++) {
        if (i != 0) {
            indexes_string += ", ";
        }

        indexes_string += std::to_string(indexes_[i]);
    }

    std::string begins_string;

    for (size_t i = 0; i < begins_.size(); i++) {
        if (i != 0) {
            begins_string += ", ";
        }

        begins_string += std::to_string(begins_[i]);
    }

    ss << "StrippedPartition {indexes=" << indexes_string
        << ", begins=" << begins_string
        << "}";

    return ss.str();
}

template <bool multithread>
StrippedPartition<multithread> StrippedPartition<multithread>::GetStrippedPartition(size_t attribute_set, const DataFrame& data) noexcept {
    static CacheWithLimit<size_t, StrippedPartition<multithread>, multithread> cache_(1e8);
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

template <bool multithread>
StrippedPartition<multithread>& StrippedPartition<multithread>::operator=(const StrippedPartition& other) {
    if (this == &other) {
        return *this;
    }
    
    indexes_ = other.indexes_;
    begins_ = other.begins_;

    return *this;
}

template class StrippedPartition<false>;
template class StrippedPartition<true>;
}
