#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "range_based_stripped_partition.h"
#include "single_attribute_predicate.h"
#include "attribute_set.h"

namespace algos::fastod {

class ComplexStrippedPartition {
private:
    std::shared_ptr<std::vector<size_t>> sp_indexes_;
    std::shared_ptr<std::vector<size_t>> sp_begins_;
    std::shared_ptr<std::vector<DataFrame::range_t>> rb_indexes_;
    std::shared_ptr<std::vector<size_t>> rb_begins_;
    bool is_stripped_partition_;
    bool should_be_converted_to_sp_;
    const DataFrame& data_;

    static constexpr inline double SMALL_RANGES_RATIO_TO_CONVERT = 0.5;
    static constexpr inline size_t MIN_MEANINGFUL_RANGE_SIZE = static_cast<size_t>(40);

    std::string sp_ToString() const;
    void sp_Product(short attribute);
    bool sp_Split(short right) const;

    std::string rb_ToString() const;
    void rb_Product(short attribute);
    bool rb_Split(short right) const;

    std::vector<DataFrame::value_indexes_t> IntersectWithAttribute(
        algos::fastod::AttributeSet::size_type attribute,
        size_t group_start,
        size_t group_end);
    
    ComplexStrippedPartition(
        const DataFrame& data,
        std::shared_ptr<std::vector<size_t>> indexes,
        std::shared_ptr<std::vector<size_t>> begins);

    ComplexStrippedPartition(
        const DataFrame& data,
        std::shared_ptr<std::vector<DataFrame::range_t>> indexes,
        std::shared_ptr<std::vector<size_t>> begins);

public:
    ComplexStrippedPartition() = delete;
    ComplexStrippedPartition(const ComplexStrippedPartition& origin) = default;

    ComplexStrippedPartition& operator=(const ComplexStrippedPartition& other);

    std::string ToString() const; 
    void Product(short attribute);
    bool Split(short right) const;

    bool ShouldBeConvertedToStrippedPartition() const;
    void ToStrippedPartition();

private:
    template <bool ascending>
    bool sp_Swap(short left, short right) const {   
        for (size_t begin_pointer = 0; begin_pointer <  sp_begins_->size() - 1; begin_pointer++) {
            size_t group_begin = sp_begins_->operator[](begin_pointer);
            size_t group_end = sp_begins_->operator[](begin_pointer + 1);

            std::vector<std::pair<int, int>> values(group_end - group_begin);

            for (size_t i = group_begin; i < group_end; ++i) {
                size_t index = sp_indexes_->operator[](i);
                values[i - group_begin] = { data_.GetValue(index, left), 
                                            data_.GetValue(index, right) };
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

    template <bool ascending>
    bool rb_Swap(short left, short right) const {   
        for (size_t begin_pointer = 0; begin_pointer <  rb_begins_->size() - 1; begin_pointer++) {
            size_t group_begin = rb_begins_->operator[](begin_pointer);
            size_t group_end = rb_begins_->operator[](begin_pointer + 1);

            std::vector<std::pair<int, int>> values;
            // values.reserve(...)?

            for (size_t i = group_begin; i < group_end; ++i) {
                DataFrame::range_t range = rb_indexes_->operator[](i);

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

public:
    template<bool ascending>
    bool Swap(short left, short right) const {
        return is_stripped_partition_
            ? sp_Swap<ascending>(left, right)
            : rb_Swap<ascending>(left, right);
    }

    template<bool range_based_mode>
    static ComplexStrippedPartition Create(const DataFrame& data) {
        if constexpr(range_based_mode) {
            std::vector<DataFrame::range_t>* rb_indexes = new std::vector<DataFrame::range_t>();
            std::vector<size_t>* rb_begins = new std::vector<size_t>();
            
            size_t tuple_count = data.GetTupleCount();
            rb_begins->push_back(0);

            if (tuple_count != 0) {
                rb_indexes->push_back({0, tuple_count - 1});
                rb_begins->push_back(1);
            }

            return ComplexStrippedPartition(
                std::move(data),
                std::shared_ptr<std::vector<DataFrame::range_t>>(rb_indexes),
                std::shared_ptr<std::vector<size_t>>(rb_begins));
        }

        std::vector<size_t>* sp_indexes = new std::vector<size_t>();
        std::vector<size_t>* sp_begins = new std::vector<size_t>();

        sp_indexes->reserve(data.GetTupleCount());
    
        for (size_t i = 0; i < data.GetTupleCount(); i++) {
            sp_indexes->push_back(i);
        }

        if (data.GetTupleCount() != 0) {
            sp_begins->push_back(0);
        }

        sp_begins->push_back(data.GetTupleCount());

        return ComplexStrippedPartition(
            std::move(data),
            std::shared_ptr<std::vector<size_t>>(sp_indexes),
            std::shared_ptr<std::vector<size_t>>(sp_begins));
    }
};

} // namespace algos::fastod
