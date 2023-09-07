#include <cstdint>
#include <sstream>
#include "stripped_partition.h"
#include "cache_with_limit.h"

namespace algos::fastod {

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

std::string StrippedPartition::ToString() const {
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

StrippedPartition& StrippedPartition::operator=(const StrippedPartition& other) {
    if (this == &other) {
        return *this;
    }
    
    indexes_ = other.indexes_;
    begins_ = other.begins_;

    return *this;
}

void StrippedPartition::Product(short attribute) {
    std::vector<size_t> new_indexes;
    new_indexes.reserve(data_.GetColumnCount());
    std::vector<size_t> new_begins;
    size_t fill_pointer = 0;

    for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
        size_t group_begin = begins_[begin_pointer];
        size_t group_end = begins_[begin_pointer + 1];
        // CHANGE: utilize column types
        std::vector<std::pair<int, size_t>> values(group_end - group_begin);

        for (size_t i = group_begin; i < group_end; i++) {
            size_t index = indexes_[i];
            values[i - group_begin] = { data_.GetValue(index, attribute), index };
        }
        std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
            return p1.first < p2.first;
        });
        size_t group_start = 0;
        size_t i = 1;
        auto addGroup = [&]() {
            size_t group_size = i - group_start;
            if (group_size > 1) {
                new_begins.push_back(fill_pointer);
                fill_pointer += group_size;
                for (size_t j = group_start; j < group_start + group_size; ++j)
                    new_indexes.push_back(values[j].second);
            }
            group_start = i;
        };
        for (; i < values.size(); ++i) {
            if (values[i - 1].first != values[i].first)
                addGroup();
        }
        addGroup();
    }

    indexes_ = std::move(new_indexes);
    begins_ = std::move(new_begins);
    begins_.push_back(indexes_.size());
}

bool StrippedPartition::Split(short right) {

    for (size_t begin_pointer = 0; begin_pointer <  begins_.size() - 1; begin_pointer++) {
        size_t group_begin = begins_[begin_pointer];
        size_t group_end = begins_[begin_pointer + 1];
        int group_value = data_.GetValue(indexes_[group_begin], right);
        for (size_t i = group_begin + 1; i < group_end; i++) {
            if (data_.GetValue(indexes_[i], right) != group_value)
                return true;
        }
    }

    return false;
}

}
