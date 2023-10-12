#pragma once

#include <vector>
#include <iostream>

#include "data_frame.h"
#include "equivalence_class.h"

namespace algos::fastod {

class DenseStrippedPartition {
private:
    std::vector<EquivalenceClass> classes_;
    const DataFrame& data_;

public:
    explicit DenseStrippedPartition(const DataFrame& data);
    DenseStrippedPartition(const DenseStrippedPartition& origin) = default;

    std::string ToString() const;
    StrippedPartition ToStrippedPartition() const;

    DenseStrippedPartition& operator=(const DenseStrippedPartition& other);

    void Product(short attribute);
    bool Split(short right);

    template <bool ascending>
    bool Swap(short left, short right) {
        //static int swap_num = 1;
        
        for (EquivalenceClass const& eq_class : classes_) {
            size_t values_fill_index = 0;
            std::vector<std::pair<int, int>> values(eq_class.Size());
            
            for (Range const& range : eq_class.GetIndexes()) {
                for (size_t i = range.GetStart(); i <= range.GetEnd(); ++i) {
                    values[values_fill_index++] = {
                        data_.GetValue(i, left), 
                        data_.GetValue(i, right)
                    };
                }
            }

            if constexpr (ascending) {
                std::sort(values.begin(), values.end(), [](const auto& x, const auto& y) {
                    return x.first < y.first;
                });
            }
            else {
                std::sort(values.begin(), values.end(), [](const auto& x, const auto& y) {
                    return y.first < x.first;
                });
            }

            bool is_first_group = true;
            size_t prev_group_max_index = 0;
            size_t curr_group_max_index = 0;
                  
            for (size_t i = 0; i < values.size(); ++i) {
                int first = values[i].first;
                int second = values[i].second;

                // values are sorted by "first"
                if (i != 0 && values[i - 1].first != first) {
                    is_first_group = false;
                    prev_group_max_index = curr_group_max_index;
                    curr_group_max_index = i;
                }
                else if (values[curr_group_max_index].second <= second) {
                    curr_group_max_index = i;
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