#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#include "range_based_stripped_partition.h"
#include "single_attribute_predicate.h"
#include "attribute_set.h"

namespace algos::fastod {

class RangeBasedStrippedPartition;

class StrippedPartition {
private:
    std::vector<size_t> indexes_;
    std::vector<size_t> begins_;
    const DataFrame& data_;

    StrippedPartition(const DataFrame& data, std::vector<size_t> const& indexes, std::vector<size_t> const& begins);
    friend class RangeBasedStrippedPartition;
    
public:
    explicit StrippedPartition(const DataFrame& data);
    StrippedPartition(const StrippedPartition& origin) = default;

    std::string ToString() const;
    StrippedPartition& operator=(const StrippedPartition& other);

    void Product(short attribute);
    bool Split(short right);

    template <bool ascending>
    bool Swap(short left, short right) {
        //static int swap_num = 1;
        
        for (size_t begin_pointer = 0; begin_pointer <  begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];
            // std::vector<std::pair<int, int>> values(group_end - group_begin);
            // for (size_t i = group_begin; i < group_end; ++i) {
            //     size_t index = indexes_[i];
            //     if constexpr (ascending) {
            //         values[i - group_begin] = { data_.GetValue(index, left),
            //                                     data_.GetValue(index, right) };
            //     } else {
            //         values[i - group_begin] = { -data_.GetValue(index, left),
            //                                     data_.GetValue(index, right) };
            //     }
            // }
            // std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
            //     return p1.first < p2.first;
            // });

            // int beforeMax = - data_.GetTupleCount() - 1;
            // int groupMax = beforeMax;
            // for (size_t i = 0; i < values.size(); ++i) {
            //     int index = values[i].first;
            //     if (i == 0 || values[i - 1].first != values[i].first) {
            //         beforeMax = std::max(groupMax, beforeMax);
            //         groupMax = index;
            //     } else {
            //         groupMax = std::max(groupMax, index);
            //     }
            //     if(index < beforeMax)
            //         return true;
            // }
            std::vector<std::pair<int, int>> values(group_end - group_begin);
            for (size_t i = group_begin; i < group_end; ++i) {
                size_t index = indexes_[i];
                values[i - group_begin] = { data_.GetValue(index, left), 
                                            data_.GetValue(index, right) };
            }

            if constexpr (ascending)
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
                    //std::cout << "Swap-" << (swap_num++) << "-" << ascending << ":\t" << "true" << std::endl;
                    return true;
                }
            }
        }
        //std::cout << "Swap-" << (swap_num++) << "-" << ascending << ":\t" << "false" << std::endl;
        return false;
    }
};

} // namespace algos::fastod
