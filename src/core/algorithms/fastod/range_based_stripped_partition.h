#pragma once

#include <vector>
#include <iostream>

#include "data_frame.h"
#include "stripped_partition.h"

namespace algos::fastod {

class StrippedPartition;

class RangeBasedStrippedPartition {
private:
    std::vector<DataFrame::range_t> indexes_;
    std::vector<size_t> begins_;
    
    const DataFrame& data_;

    std::vector<DataFrame::value_indexes_t> IntersectWithAttribute(
        algos::fastod::AttributeSet::size_type attribute,
        size_t group_start,
        size_t group_end);
    
public:
    explicit RangeBasedStrippedPartition(const DataFrame& data);
    RangeBasedStrippedPartition(const RangeBasedStrippedPartition& origin) = default;

    std::string ToString() const;
    StrippedPartition ToStrippedPartition() const;

    RangeBasedStrippedPartition& operator=(const RangeBasedStrippedPartition& other);

    void Product(short attribute);
    bool Split(short right);

    template <bool ascending>
    bool Swap(short left, short right) {
        //static int swap_num = 1;
        
        for (size_t begin_pointer = 0; begin_pointer <  begins_.size() - 1; begin_pointer++) {
            size_t group_begin = begins_[begin_pointer];
            size_t group_end = begins_[begin_pointer + 1];

            std::vector<std::pair<int, int>> values;
            // values.reserve(...)?

            for (size_t i = group_begin; i < group_end; ++i) {
                DataFrame::range_t range = indexes_[i];

                for (size_t j = range.first; j <= range.second; ++j) {
                    values.push_back({
                        data_.GetValue(j, left), 
                        data_.GetValue(j, right)
                    });
                }
            }

            if constexpr (ascending) {
                std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
                    return p1.first < p2.first;
                });
            }
            else {
                std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
                    return p2.first < p1.first;
                });
            }

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