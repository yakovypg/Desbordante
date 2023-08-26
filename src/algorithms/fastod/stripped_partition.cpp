#include <optional>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <sstream>

#include "schema_value.h"
#include "value_pair.h"
#include "operator.h"
#include "operator_type.h"
#include "stripped_partition.h"
// #include "timer.h"

using namespace algos::fastod;

// important
CacheWithLimit<size_t, StrippedPartition> StrippedPartition::cache_(1e8);

StrippedPartition::StrippedPartition() : indexes_({}), begins_({}), data_(DataFrame()) { }

StrippedPartition::StrippedPartition(const DataFrame& data) noexcept : data_(std::move(data)) {
    indexes_.reserve(data.GetTupleCount());
    for (int i = 0; i < data.GetTupleCount(); i++) {
        indexes_.push_back(i);
    }

    if (data.GetTupleCount() != 0) {
        begins_.push_back(0);
    }

    begins_.push_back(data.GetTupleCount());
}

StrippedPartition::StrippedPartition(StrippedPartition const &origin) noexcept : data_(origin.data_) {
    indexes_ = origin.indexes_;
    begins_ = origin.begins_;
}

void StrippedPartition::Product(int attribute) noexcept {

    std::vector<int> new_indexes;
    std::vector<int> new_begins;
    int fill_pointer = 0;

    for (int begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
        int group_begin = begins_[begin_pointer];
        int group_end = begins_[begin_pointer + 1];
        // CHANGE: utilize column types
        std::unordered_map<SchemaValue, std::vector<int>> subgroups;

        for (int i = group_begin; i < group_end; i++) {
            int index = indexes_[i];
            const auto& value = data_.GetValue(index, attribute);

            if (subgroups.count(value) == 0)
                subgroups[value] = {};

            subgroups[value].push_back(index);
        }

        new_begins.reserve(new_begins.size() + subgroups.size());
        for (auto& [_, new_group]: subgroups){
            if (new_group.size() > 1){
                new_begins.push_back(fill_pointer);
                fill_pointer += new_group.size();
                new_indexes.insert(new_indexes.end(), new_group.begin(), new_group.end());
            }
        }
    }

    indexes_ = std::move(new_indexes);
    begins_ = std::move(new_begins);
    begins_.push_back(indexes_.size());
}


bool StrippedPartition::Split(int right) noexcept {

    for (int begin_pointer = 0; begin_pointer <  begins_.size() - 1; begin_pointer++) {
        int group_begin = begins_[begin_pointer];
        int group_end = begins_[begin_pointer + 1];
        const auto& group_value = data_.GetValue(indexes_[group_begin], right);

        for (int i = group_begin + 1; i < group_end; i++) {
            int index = indexes_[i];
            const auto& value = data_.GetValue(index, right);

            if (value != group_value) {

                return true;
            }
        }
    }

    return false;
}

bool StrippedPartition::Swap(const SingleAttributePredicate& left, int right) noexcept {
    for (int begin_pointer = 0; begin_pointer <  begins_.size() - 1; begin_pointer++) {
        int group_begin = begins_[begin_pointer];
        int group_end = begins_[begin_pointer + 1];
        std::vector<ValuePair> values;
        values.reserve(group_end - group_begin);
        for (int i = group_begin; i < group_end; i++) {
            int index = indexes_[i];

            values.emplace_back(
                data_.GetValue(index, left.GetAttribute()),
                data_.GetValue(index, right)
            );
        }
        // CHANGE: utilize operators
        // SCOPE: from here until the end of this loop

        if (left.GetOperator().GetType() != OperatorType::GreaterOrEqual)
            std::sort(values.begin(), values.end());
        else
            std::sort(values.begin(), values.end(), std::greater<>());

        int prev_group_max_index = 0;
        int current_group_max_index = 0;
        bool is_first_group = true;
        
        
        for (int i = 0; i < values.size(); i++) {
            const auto& first = values[i].GetFirst();
            const auto& second = values[i].GetSecond();

            // values are sorted by "first"
            if (i != 0 && values[i - 1].GetFirst() != first) {
                is_first_group = false;
                prev_group_max_index = current_group_max_index;
                current_group_max_index = i;
            }

            if (values[current_group_max_index].GetSecond() <= second) {
                current_group_max_index = i;
            }

            if (!is_first_group && values[prev_group_max_index].GetSecond() > second) {
                return true;
            }
        }     
    }

    return false;
}

std::string StrippedPartition::ToString() const noexcept {
    std::stringstream ss;
    std::string indexes_string;

    for (int i = 0; i < indexes_.size(); i++) {
        if (i != 0) {
            indexes_string += ", ";
        }

        indexes_string += std::to_string(indexes_[i]);
    }

    std::string begins_string;

    for (int i = 0; i < begins_.size(); i++) {
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

StrippedPartition StrippedPartition::DeepClone() const noexcept {
    StrippedPartition result(data_);
    result.indexes_ = indexes_;
    result.begins_ = begins_;
    return result;
}

StrippedPartition StrippedPartition::GetStrippedPartition(size_t attribute_set, const DataFrame& data) noexcept {
    if (StrippedPartition::cache_.Contains(attribute_set)) {
        return StrippedPartition::cache_.Get(attribute_set);
    }

    std::optional<StrippedPartition> result;

    for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
        size_t one_less = deleteAttribute(attribute_set, *attr);
        
        if (StrippedPartition::cache_.Contains(one_less)) {
            result = StrippedPartition::cache_.Get(one_less).DeepClone();
            result->Product(*attr);
        }
    }

    if (!result) {
        result = StrippedPartition(data);

        for (ASIterator attr = attrsBegin(attribute_set); attr != attrsEnd(attribute_set); ++attr) {
            result->Product(*attr);
        }
    }

    StrippedPartition::cache_.Set(attribute_set, *result);

    return result.value();
}

long StrippedPartition::SplitRemoveCount(int right) noexcept {
    long result = 0;

    for (int begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
        int group_begin = begins_[begin_pointer];
        int group_end = begins_[begin_pointer + 1];
        int group_length = group_end - group_begin;
        // CHANGE: key type according to column types
        std::unordered_map<SchemaValue, int> group_int_2_count;

        for (int i = group_begin; i < group_end; i++) {
            const auto& right_value = data_.GetValue(indexes_[i], right);
            
            if (group_int_2_count.count(right_value) != 0) {
                ++group_int_2_count[right_value];
            } else {
                group_int_2_count[right_value] = 1;
            }
        }

        int max = INT32_MIN;

        for (auto const& [_, count] : group_int_2_count) {
            max = std::max(max, count);
        }

        result += group_length - max;
    }

    return result;
}

long StrippedPartition::SwapRemoveCount(const SingleAttributePredicate& left, int right) noexcept {
    std::size_t length = indexes_.size();
    std::vector<int> violations_count(length);
    std::vector<bool> deleted(length);
    int result = 0;

next_class:
    for (int begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
        int group_begin = begins_[begin_pointer];
        int group_end = begins_[begin_pointer + 1];

        for (int i = group_begin; i < group_end; i++) {
            // CHANGE: was FiltereDataFrameGet
            const auto& left_i = data_.GetValue(indexes_[i], left.GetAttribute());
            const auto& right_i = data_.GetValue(indexes_[i], right);

            for (int j = i + 1; j < group_end; j++) {
                // CHANGE: was FiltereDataFrameGet
                const auto& left_j = data_.GetValue(indexes_[j], left.GetAttribute());
                const auto& right_j = data_.GetValue(indexes_[j], right);

                // CHANGE: this comparison now uses operators
                // this is needed to get rid of FilteredDataFrameGet
                if (left_i != left_j
                    && right_i != right_j
                    && left.GetOperator().Satisfy(left_i, left_j)
                    && left.GetOperator().Violate(right_i, right_j)
                ) {
                    violations_count[i]++;
                    violations_count[j]++;
                }
            }
        }

        while (true) {
            int delete_index = -1;

            for (int i = group_begin; i < group_end; i++) {
                if (!deleted[i] && (delete_index == -1 || violations_count[i] > violations_count[delete_index])) {
                    delete_index = i;
                }
            }

            if (delete_index == -1 || violations_count[delete_index] == 0) {
                goto next_class;
            }

            result++;
            deleted[delete_index] = true;

            // CHANGE: was FiltereDataFrameGet
            const auto& left_i = data_.GetValue(indexes_[delete_index], left.GetAttribute());
            const auto& right_i = data_.GetValue(indexes_[delete_index], right);

            for (int j = group_begin; j < group_end; j++) {
                // CHANGE: was FiltereDataFrameGet
                const auto& left_j = data_.GetValue(indexes_[j], left.GetAttribute());
                const auto& right_j = data_.GetValue(indexes_[j], right);

                // CHANGE: this comparison now uses operators
                // this is needed to get rid of FilteredDataFrameGet
                if (left_i != left_j
                    && right_i != right_j
                    && left.GetOperator().Satisfy(left_i, left_j)
                    && left.GetOperator().Violate(right_i, right_j)
                ) {
                    violations_count[j]--;
                }
            }
        }
    }

    return result;
}

StrippedPartition& StrippedPartition::operator=(const StrippedPartition& other) {
    if (this == &other) {
        return *this;
    }
    
    indexes_ = other.indexes_;
    begins_ = other.begins_;

    return *this;
}

