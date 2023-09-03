#include <cstdint>
#include <sstream>
#include "stripped_partition.h"
#include "cache_with_limit.h"

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
