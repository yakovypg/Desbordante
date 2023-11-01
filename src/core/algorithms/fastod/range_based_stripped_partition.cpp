#include "stripped_partition.h"
#include "range_based_stripped_partition.h"

#include <iostream>
#include <sstream>

namespace algos::fastod {

RangeBasedStrippedPartition::RangeBasedStrippedPartition(const DataFrame& data)
    : data_(std::move(data)) {
    size_t tuple_count = data.GetTupleCount();
    begins_.push_back(0);

    if (tuple_count != 0) {
        indexes_.push_back({0, tuple_count - 1});
        begins_.push_back(1);
    }
}

std::string RangeBasedStrippedPartition::ToString() const {
    std::stringstream ss;
    std::string indexes_string;

    for (size_t i = 0; i < indexes_.size(); i++) {
        if (i != 0) {
            indexes_string += ", ";
        }

        indexes_string += "(" + std::to_string(indexes_[i].first) + ";" +
                          std::to_string(indexes_[i].second) + ")";
    }

    std::string begins_string;

    for (size_t i = 0; i < begins_.size(); i++) {
        if (i != 0) {
            begins_string += ", ";
        }

        begins_string += std::to_string(begins_[i]);
    }

    ss << "RangeBasedStrippedPartition { indexes = [ " << indexes_string << " ]; begins = [ "
       << begins_string << " ] }";

    return ss.str();
}

StrippedPartition RangeBasedStrippedPartition::ToStrippedPartition() const {
    std::vector<size_t> sp_indexes;
    std::vector<size_t> sp_begins;

    size_t sp_begin = 0;
    sp_begins.push_back(sp_begin);

    for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; begin_pointer++) {
        size_t group_begin = begins_[begin_pointer];
        size_t group_end = begins_[begin_pointer + 1];

        for (size_t i = group_begin; i < group_end; ++i) {
            DataFrame::range_t range = indexes_[i];
            sp_begin += range.second - range.first + 1;

            for (size_t sp_index = range.first; sp_index <= range.second; ++sp_index) {
                sp_indexes.push_back(sp_index);
            }
        }

        sp_begins.push_back(sp_begin);
    }

    return StrippedPartition(data_, std::move(sp_indexes), std::move(sp_begins));
}

RangeBasedStrippedPartition& RangeBasedStrippedPartition::operator=(
        const RangeBasedStrippedPartition& other) {
    if (this == &other) {
        return *this;
    }

    indexes_ = other.indexes_;
    begins_ = other.begins_;

    return *this;
}

void RangeBasedStrippedPartition::Product(short attribute) {
    // static int product_index = 1;

    // if (product_index == 38) {
    //     std::cout << "";
    // }

    std::vector<size_t> new_begins;
    std::vector<DataFrame::range_t> new_indexes;

    size_t curr_begin = 0;

    for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; ++begin_pointer) {
        size_t group_begin = begins_[begin_pointer];
        size_t group_end = begins_[begin_pointer + 1];

        std::vector<DataFrame::value_indexes_t> intersection =
                IntersectWithAttribute(attribute, group_begin, group_end - 1);

        size_t intersection_size = intersection.size();

        auto AddGroup = [&new_indexes, &new_begins, &intersection, &curr_begin](size_t start_index,
                                                                                size_t end_index) {
            if (start_index == end_index) {
                DataFrame::range_t range = intersection[start_index].second;

                if (range.second == range.first) {
                    return;
                }
            }

            for (size_t i = start_index; i <= end_index; ++i) {
                new_indexes.push_back(intersection[i].second);
            }

            new_begins.push_back(curr_begin);
            curr_begin += end_index - start_index + 1;
        };

        size_t group_start = 0;

        for (size_t i = 1; i < intersection_size; ++i) {
            if (intersection[i].first != intersection[i - 1].first) {
                AddGroup(group_start, i - 1);
                group_start = i;
            }
        }

        AddGroup(group_start, intersection_size - 1);
    }

    new_begins.push_back(new_indexes.size());

    indexes_ = std::move(new_indexes);
    begins_ = std::move(new_begins);

    //std::cout << product_index++ << "-" << ToStrippedPartition().ToString() << std::endl;
}

bool RangeBasedStrippedPartition::Split(short right) {
    for (size_t begin_pointer = 0; begin_pointer < begins_.size() - 1; ++begin_pointer) {
        size_t group_begin = begins_[begin_pointer];
        size_t group_end = begins_[begin_pointer + 1];

        int group_value = data_.GetValue(indexes_[group_begin].first, right);

        for (size_t i = group_begin; i < group_end; ++i) {
            DataFrame::range_t range = indexes_[i];

            for (size_t j = range.first; j <= range.second; ++j) {
                if (data_.GetValue(j, right) != group_value) {
                    // std::cout << "Split:\t"
                    //           << "true" << std::endl;
                    return true;
                }
            }
        }
    }

    // std::cout << "Split:\t"
    //           << "false" << std::endl;
    return false;
}

std::vector<DataFrame::value_indexes_t> RangeBasedStrippedPartition::IntersectWithAttribute(
        algos::fastod::AttributeSet::size_type attribute, size_t group_start, size_t group_end) {
    std::vector<DataFrame::value_indexes_t> result;
    std::vector<DataFrame::value_indexes_t> const& attr_ranges =
            data_.GetDataRanges().at(attribute);

    for (size_t i = group_start; i <= group_end; ++i) {
        DataFrame::range_t const& range = indexes_[i];

        size_t lower_bound_range_index = data_.GetRangeIndexByItem(range.first, attribute);
        size_t upper_bound_range_index = data_.GetRangeIndexByItem(range.second, attribute);

        for (size_t j = lower_bound_range_index; j <= upper_bound_range_index; ++j) {
            DataFrame::value_indexes_t const& attr_value_range = attr_ranges.at(j);
            DataFrame::range_t const& attr_range = attr_value_range.second;

            size_t start = std::max(range.first, attr_range.first);
            size_t end = std::min(range.second, attr_range.second);

            if (start <= end) {
                result.push_back({attr_value_range.first, {start, end}});
            }
        }
    }

    std::sort(result.begin(), result.end(),
              [](auto const& x, auto const& y) { return x.first < y.first; });

    return result;
}

// std::vector<DataFrame::value_indexes_t>
// RangeBasedStrippedPartition::IntersectWithAttribute(algos::fastod::AttributeSet::size_type
// attribute) {
//     std::vector<DataFrame::value_indexes_t> result;
//     std::vector<DataFrame::value_indexes_t> const& attr_ranges =
//     data_.GetDataRanges().at(attribute);

//     for (DataFrame::range_t const& range : indexes_) {
//         size_t lower_bound_range_index = data_.GetRangeIndexByItem(range.first, attribute);
//         size_t upper_bound_range_index = data_.GetRangeIndexByItem(range.second, attribute);

//         for (size_t i = lower_bound_range_index; i <= upper_bound_range_index; ++i) {
//             DataFrame::value_indexes_t const& attr_value_range = attr_ranges.at(i);
//             DataFrame::range_t const& attr_range = attr_value_range.second;

//             size_t start = std::max(range.first, attr_range.first);
//             size_t end = std::min(range.second, attr_range.second);

//             if (start <= end) {
//                 result.push_back({
//                     attr_value_range.first,
//                     { start, end }
//                 });
//             }
//         }
//     }

//     std::sort(result.begin(), result.end(), [](auto const& x, auto const& y) {
//         return x.first < y.first;
//     });

//     return result;
// }

}  // namespace algos::fastod