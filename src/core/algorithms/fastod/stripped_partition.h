#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#include "single_attribute_predicate.h"
#include "attribute_set.h"

namespace algos::fastod {

template <bool multithread>
class StrippedPartition;

using SingleStrippedPartition = StrippedPartition<false>;
using MultiStrippedPartition = StrippedPartition<true>;

template <bool multithread>
class StrippedPartition {
private:
    std::vector<size_t> indexes_;
    std::vector<size_t> begins_;
    const DataFrame& data_;
    
public:
    explicit StrippedPartition(const DataFrame& data);
    StrippedPartition(const StrippedPartition& origin) = default;

    template <typename T>
    void Product(size_t attribute) noexcept {

        std::vector<size_t> new_indexes;
        new_indexes.reserve(data_.GetColumnCount());
        std::vector<size_t> new_begins;
        size_t fill_pointer = 0;

        for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];
            // CHANGE: utilize column types
            std::vector<std::pair<T, size_t>> values(group_end - group_begin);

            for (size_t i = group_begin; i < group_end; i++) {
                size_t index = indexes_[i];
                values[i - group_begin] = { data_.GetValue<T>(index, attribute), index };
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

            // std::unordered_map<T, std::vector<size_t>> subgroups;

            // for (size_t i = group_begin; i < group_end; i++) {
            //     size_t index = indexes_[i];
            //     subgroups[data_.GetValue<T>(index, attribute)].push_back(index);
            // }
            // new_begins.reserve(new_begins.size() + subgroups.size());
            // for (auto& [_, new_group]: subgroups) {
            //     if (new_group.size() > 1) {
            //         new_begins.push_back(fill_pointer);
            //         fill_pointer += new_group.size();
            //         new_indexes.insert(new_indexes.end(), new_group.begin(), new_group.end());
            //     }
            // }
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
            std::vector<std::pair<TL, TR>> values(group_end - group_begin);
            for (size_t i = group_begin; i < group_end; ++i) {
                size_t index = indexes_[i];
                values[i - group_begin] = { data_.GetValue<TL>(index, left.attribute), 
                                            data_.GetValue<TR>(index, right) };
            }
            // CHANGE: utilize operators
            // SCOPE: from here until the end of this loop

            if (left.ascending)
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
                typename constResType<TL>::type left_i = data_.GetValue<TL>(indexes_[i], left.attribute);
                typename constResType<TR>::type right_i = data_.GetValue<TR>(indexes_[i], right);

                for (size_t j = i + 1; j < group_end; j++) {
                    // CHANGE: was FiltereDataFrameGet
                    typename constResType<TL>::type left_j = data_.GetValue<TL>(indexes_[j], left.attribute);
                    typename constResType<TR>::type right_j = data_.GetValue<TR>(indexes_[j], right);

                    // CHANGE: this comparison now uses operators
                    // this is needed to get rid of FilteredDataFrameGet
                    if (left_i != left_j
                        && right_i != right_j
                        && left.Satisfy<TL>(left_i, left_j)
                        && left.Satisfy<TR>(right_j, right_i)
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
                typename constResType<TL>::type left_i = data_.GetValue<TL>(indexes_[delete_index], left.attribute);
                typename constResType<TR>::type right_i = data_.GetValue<TR>(indexes_[delete_index], right);

                for (size_t j = group_begin; j < group_end; j++) {
                    // CHANGE: was FiltereDataFrameGet
                    typename constResType<TL>::type left_j = data_.GetValue<TL>(indexes_[j], left.attribute);
                    typename constResType<TR>::type right_j = data_.GetValue<TR>(indexes_[j], right);

                    // CHANGE: this comparison now uses operators
                    // this is needed to get rid of FilteredDataFrameGet
                    if (left_i != left_j
                        && right_i != right_j
                        && left.Satisfy<TL>(left_i, left_j)
                        && left.Satisfy<TR>(right_j, right_i)
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
