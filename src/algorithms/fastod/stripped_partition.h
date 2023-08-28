#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <optional>

#include "data_frame.h"
#include "schema_value.h"
#include "single_attribute_predicate.h"
#include "cache_with_limit.h"
#include "attribute_set.h"
// #include "value_pair.h"
#include "operator.h"
#include "operator_type.h"

namespace algos::fastod {

class StrippedPartition {
private:
    std::vector<size_t> indexes_;
    std::vector<size_t> begins_;
    const DataFrame& data_;
    static CacheWithLimit<size_t, StrippedPartition> cache_;
    
public:
    explicit StrippedPartition(const DataFrame& data);
    StrippedPartition(const StrippedPartition& origin) = default;

    template <typename T>
    void Product(size_t attribute) noexcept {

        std::vector<size_t> new_indexes;
        std::vector<size_t> new_begins;
        size_t fill_pointer = 0;

        for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];
            // CHANGE: utilize column types
            std::unordered_map<T, std::vector<size_t>> subgroups;

            for (size_t i = group_begin; i < group_end; i++) {
                size_t index = indexes_[i];
                typename constResType<T>::type value = data_.GetValue<T>(index, attribute);

                if (subgroups.count(value) == 0)
                    subgroups[value] = {};

                subgroups[value].push_back(index);
            }

            new_begins.reserve(new_begins.size() + subgroups.size());
            for (auto& [_, new_group]: subgroups) {
                if (new_group.size() > 1) {
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

    template <typename TR>
    bool Split(size_t right) noexcept {

        for (size_t begin_pointer = 0; begin_pointer <  begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];
            typename constResType<TR>::type group_value = data_.GetValue<TR>(indexes_[group_begin], right);
            for (size_t i = group_begin + 1; i < group_end; i++) {
                if (data_.GetValue<TR>(indexes_[i], right) != group_value)
                    return true;
            }
        }

        return false;
    }

    template <typename TL, typename TR>
    bool Swap(const SingleAttributePredicate& left, size_t right) noexcept {
        for (size_t begin_pointer = 0; begin_pointer <  begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];
            std::vector<std::pair<TL, TR>> values;
            values.reserve(group_end - group_begin);
            for (size_t i = group_begin; i < group_end; i++) {
                size_t index = indexes_[i];
                values.emplace_back(
                    data_.GetValue<TL>(index, left.GetAttribute()),
                    data_.GetValue<TR>(index, right)
                );
            }
            // CHANGE: utilize operators
            // SCOPE: from here until the end of this loop

            if (left.GetOperator().GetType() != OperatorType::GreaterOrEqual)
                std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
                    return p1.first < p2.first;
                });
            else
                std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
                    return p2.first < p1.first;
                });

            size_t prev_group_max_index = 0;
            size_t current_group_max_index = 0;
            bool is_first_group = true;
            
            
            for (size_t i = 0; i < values.size(); i++) {
                const auto& first = values[i].first;
                const auto& second = values[i].second;

                // values are sorted by "first"
                if (i != 0 && values[i - 1].first != first) {
                    is_first_group = false;
                    prev_group_max_index = current_group_max_index;
                    current_group_max_index = i;
                } else if (values[current_group_max_index].second <= second) {
                    current_group_max_index = i;
                }

                if (!is_first_group && values[prev_group_max_index].second > second) {
                    return true;
                }
            }
        }

        return false;
    }

    std::string ToString() const noexcept;
    static StrippedPartition GetStrippedPartition(size_t attribute_set, const DataFrame& data) noexcept;

    template <typename TR>
    long SplitRemoveCount(size_t right) noexcept {
        long result = 0;

        for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];
            size_t group_length = group_end - group_begin;
            // CHANGE: key type according to column types
            std::unordered_map<TR, size_t> group_int_2_count;

            for (size_t i = group_begin; i < group_end; i++) {
                typename constResType<TR>::type right_value = data_.GetValue<TR>(indexes_[i], right);
                
                if (group_int_2_count.count(right_value) != 0)
                    ++group_int_2_count[right_value];
                else
                    group_int_2_count[right_value] = 1;
            }

            size_t max = 0;
            for (auto const& [_, count] : group_int_2_count) {
                max = std::max(max, count);
            }

            result += group_length - max;
        }

        return result;
    }
    template <typename TL, typename TR>
    long SwapRemoveCount(const SingleAttributePredicate& left, size_t right) noexcept {
        std::size_t length = indexes_.size();
        std::vector<size_t> violations_count(length);
        std::vector<bool> deleted(length);
        size_t result = 0;

    next_class:
        for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];

            for (size_t i = group_begin; i < group_end; i++) {
                // CHANGE: was FiltereDataFrameGet
                typename constResType<TL>::type left_i = data_.GetValue<TL>(indexes_[i], left.GetAttribute());
                typename constResType<TR>::type right_i = data_.GetValue<TR>(indexes_[i], right);

                for (size_t j = i + 1; j < group_end; j++) {
                    // CHANGE: was FiltereDataFrameGet
                    typename constResType<TL>::type left_j = data_.GetValue<TL>(indexes_[j], left.GetAttribute());
                    typename constResType<TR>::type right_j = data_.GetValue<TR>(indexes_[j], right);

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

                for (size_t i = group_begin; i < group_end; i++) {
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
                typename constResType<TL>::type left_i = data_.GetValue<TL>(indexes_[delete_index], left.GetAttribute());
                typename constResType<TR>::type right_i = data_.GetValue<TR>(indexes_[delete_index], right);

                for (size_t j = group_begin; j < group_end; j++) {
                    // CHANGE: was FiltereDataFrameGet
                    typename constResType<TL>::type left_j = data_.GetValue<TL>(indexes_[j], left.GetAttribute());
                    typename constResType<TR>::type right_j = data_.GetValue<TR>(indexes_[j], right);

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

    StrippedPartition& operator=(const StrippedPartition& other);
};

} // namespace algos::fastod
