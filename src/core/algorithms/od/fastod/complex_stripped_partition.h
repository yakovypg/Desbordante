#pragma once

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

#include "attribute_set.h"
#include "range_based_stripped_partition.h"

namespace algos::fastod {

class ComplexStrippedPartition {
private:
    std::shared_ptr<std::vector<size_t>> sp_indexes_;
    std::shared_ptr<std::vector<size_t>> sp_begins_;
    std::shared_ptr<std::vector<DataFrame::range_t>> rb_indexes_;
    std::shared_ptr<std::vector<size_t>> rb_begins_;
    std::shared_ptr<DataFrame> data_;
    bool is_stripped_partition_;
    bool should_be_converted_to_sp_;

    static constexpr inline double SMALL_RANGES_RATIO_TO_CONVERT = 0.5;
    static constexpr inline size_t MIN_MEANINGFUL_RANGE_SIZE = static_cast<size_t>(40);

    std::string sp_ToString() const;
    void sp_Product(short attribute);
    bool sp_Split(short right) const;

    std::string rb_ToString() const;
    void rb_Product(short attribute);
    bool rb_Split(short right) const;

    std::vector<DataFrame::value_indexes_t> IntersectWithAttribute(
            algos::fastod::AttributeSet::size_type attribute, size_t group_start, size_t group_end);

    ComplexStrippedPartition(std::shared_ptr<DataFrame> data,
                             std::shared_ptr<std::vector<size_t>> indexes,
                             std::shared_ptr<std::vector<size_t>> begins);

    ComplexStrippedPartition(std::shared_ptr<DataFrame> data,
                             std::shared_ptr<std::vector<DataFrame::range_t>> indexes,
                             std::shared_ptr<std::vector<size_t>> begins);

public:
    ComplexStrippedPartition();
    ComplexStrippedPartition(ComplexStrippedPartition const& origin) = default;

    ComplexStrippedPartition& operator=(ComplexStrippedPartition const& other);

    std::string ToString() const;
    void Product(short attribute);
    bool Split(short right) const;

    bool ShouldBeConvertedToStrippedPartition() const;
    void ToStrippedPartition();

    template <bool ascending>
    bool Swap(short left, short right) const {
        const size_t group_count = is_stripped_partition_ ? sp_begins_->size() : rb_begins_->size();

        for (size_t begin_pointer = 0; begin_pointer < group_count - 1; begin_pointer++) {
            const size_t group_begin = is_stripped_partition_ ? (*sp_begins_)[begin_pointer]
                                                              : (*rb_begins_)[begin_pointer];

            const size_t group_end = is_stripped_partition_ ? (*sp_begins_)[begin_pointer + 1]
                                                            : (*rb_begins_)[begin_pointer + 1];

            std::vector<std::pair<int, int>> values;

            if (is_stripped_partition_) {
                values.reserve(group_end - group_begin);
            }

            if (is_stripped_partition_) {
                for (size_t i = group_begin; i < group_end; ++i) {
                    const size_t index = (*sp_indexes_)[i];

                    values.emplace_back(data_->GetValue(index, left),
                                        data_->GetValue(index, right));
                }
            } else {
                for (size_t i = group_begin; i < group_end; ++i) {
                    const DataFrame::range_t range = (*rb_indexes_)[i];

                    for (size_t j = range.first; j <= range.second; ++j) {
                        values.emplace_back(data_->GetValue(j, left), data_->GetValue(j, right));
                    }
                }
            }

            if constexpr (ascending) {
                std::sort(values.begin(), values.end(),
                          [](auto const& p1, auto const& p2) { return p1.first < p2.first; });
            } else {
                std::sort(values.begin(), values.end(),
                          [](auto const& p1, auto const& p2) { return p2.first < p1.first; });
            }

            size_t prev_group_max_index = 0;
            size_t current_group_max_index = 0;
            bool is_first_group = true;

            for (size_t i = 0; i < values.size(); i++) {
                auto const& first = values[i].first;
                auto const& second = values[i].second;

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

    template <bool range_based_mode>
    static ComplexStrippedPartition Create(std::shared_ptr<DataFrame> data) {
        if constexpr (range_based_mode) {
            std::vector<DataFrame::range_t>* rb_indexes = new std::vector<DataFrame::range_t>();
            std::vector<size_t>* rb_begins = new std::vector<size_t>();

            const size_t tuple_count = data->GetTupleCount();
            rb_begins->push_back(0);

            if (tuple_count != 0) {
                rb_indexes->push_back({0, tuple_count - 1});
                rb_begins->push_back(1);
            }

            return ComplexStrippedPartition(
                    std::move(data), std::shared_ptr<std::vector<DataFrame::range_t>>(rb_indexes),
                    std::shared_ptr<std::vector<size_t>>(rb_begins));
        }

        std::vector<size_t>* sp_indexes = new std::vector<size_t>();
        std::vector<size_t>* sp_begins = new std::vector<size_t>();

        sp_indexes->reserve(data->GetTupleCount());

        for (size_t i = 0; i < data->GetTupleCount(); i++) {
            sp_indexes->push_back(i);
        }

        if (data->GetTupleCount() != 0) {
            sp_begins->push_back(0);
        }

        sp_begins->push_back(data->GetTupleCount());

        return ComplexStrippedPartition(std::move(data),
                                        std::shared_ptr<std::vector<size_t>>(sp_indexes),
                                        std::shared_ptr<std::vector<size_t>>(sp_begins));
    }
};

}  // namespace algos::fastod
