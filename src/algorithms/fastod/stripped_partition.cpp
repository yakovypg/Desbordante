#include <cstdint>
#include <sstream>
#include "stripped_partition.h"
// #include "timer.h"

using namespace algos::fastod;

// important
CacheWithLimit<size_t, StrippedPartition> StrippedPartition::cache_(1e8);

StrippedPartition::StrippedPartition(const DataFrame& data) : data_(std::move(data)) {
    indexes_.reserve(data.GetTupleCount());
    for (size_t i = 0; i < data.GetTupleCount(); i++) {
        indexes_.push_back(i);
    }

    if (data.GetTupleCount() != 0) {
        begins_.push_back(0);
    }

    begins_.push_back(data.GetTupleCount());
}

std::string StrippedPartition::ToString() const noexcept {
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

StrippedPartition StrippedPartition::GetStrippedPartition(size_t attribute_set, const DataFrame& data) noexcept {
    if (StrippedPartition::cache_.Contains(attribute_set)) {
        return StrippedPartition::cache_.Get(attribute_set);
    }

    std::optional<StrippedPartition> result;

    auto callProduct = [&result, &data](size_t attr) {
        if (data.GetColTypeId(attr) == +model::TypeId::kInt)
            result->Product<int>(attr);
        else if (data.GetColTypeId(attr) == +model::TypeId::kDouble)
            result->Product<double>(attr);
        else
            result->Product<std::string>(attr);
    };

    for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
        size_t one_less = deleteAttribute(attribute_set, *attr);

        if (StrippedPartition::cache_.Contains(one_less)) {
            result = StrippedPartition::cache_.Get(one_less);
            callProduct(*attr);
        }
    }

    if (!result) {
        result = StrippedPartition(data);

        for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
            callProduct(*attr);
        }
    }

    StrippedPartition::cache_.Set(attribute_set, *result);

    return std::move(result.value());
}

StrippedPartition& StrippedPartition::operator=(const StrippedPartition& other) {
    if (this == &other) {
        return *this;
    }
    
    indexes_ = other.indexes_;
    begins_ = other.begins_;

    return *this;
}

