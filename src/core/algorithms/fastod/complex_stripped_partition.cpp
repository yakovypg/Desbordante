#include <cstdint>
#include <sstream>

#include "complex_stripped_partition.h"
#include "cache_with_limit.h"

namespace algos::fastod {

ComplexStrippedPartition::ComplexStrippedPartition(const DataFrame& data, std::shared_ptr<std::vector<size_t>> indexes,
    std::shared_ptr<std::vector<size_t>> begins)
    : sp_indexes_(indexes), sp_begins_(begins), is_stripped_partition_(true), data_(data) {}

ComplexStrippedPartition::ComplexStrippedPartition(const DataFrame& data,
    std::shared_ptr<std::vector<DataFrame::range_t>> indexes, std::shared_ptr<std::vector<size_t>> begins)
    : rb_indexes_(indexes), rb_begins_(begins), is_stripped_partition_(false), data_(data) {}

ComplexStrippedPartition& ComplexStrippedPartition::operator=(const ComplexStrippedPartition& other) {
    if (this == &other) {
        return *this;
    }
    
    sp_indexes_ = other.sp_indexes_;
    sp_begins_ = other.sp_begins_;
    rb_indexes_ = other.rb_indexes_;
    rb_begins_ = other.rb_begins_;
    
    should_be_converted_to_sp_ = other.should_be_converted_to_sp_;
    is_stripped_partition_ = other.is_stripped_partition_;

    return *this;
}

std::string ComplexStrippedPartition::ToString() const {
    return is_stripped_partition_
        ? sp_ToString()
        : rb_ToString();
}

void ComplexStrippedPartition::Product(short attribute) {
    if (is_stripped_partition_) {
        sp_Product(attribute);
    }
    else {
        rb_Product(attribute);
    }
}

bool ComplexStrippedPartition::Split(short right) const {
    return is_stripped_partition_
        ? sp_Split(right)
        : rb_Split(right);
}

bool ComplexStrippedPartition::ShouldBeConvertedToStrippedPartition() const {
    return should_be_converted_to_sp_;
}

void ComplexStrippedPartition::ToStrippedPartition() {
    sp_begins_ = std::shared_ptr<std::vector<size_t>>(new std::vector<size_t>());
    sp_indexes_ = std::shared_ptr<std::vector<size_t>>(new std::vector<size_t>());
    
    size_t sp_begin = 0;
    sp_begins_->push_back(sp_begin);

    for (size_t begin_pointer = 0; begin_pointer < rb_begins_->size() - 1; begin_pointer++) {
        size_t group_begin = rb_begins_->operator[](begin_pointer);
        size_t group_end = rb_begins_->operator[](begin_pointer + 1);

        for (size_t i = group_begin; i < group_end; ++i) {
            DataFrame::range_t range = rb_indexes_->operator[](i);
            sp_begin += range_size(range);

            for (size_t sp_index = range.first; sp_index <= range.second; ++sp_index) {
                sp_indexes_->push_back(sp_index);
            }
        }

        sp_begins_->push_back(sp_begin);
    }

    rb_begins_->clear();
    rb_indexes_->clear();

    is_stripped_partition_ = true;
    should_be_converted_to_sp_ = false;
}

std::string ComplexStrippedPartition::sp_ToString() const {
    std::stringstream ss;
    std::string indexes_string;

    for (size_t i = 0; i < sp_indexes_->size(); i++) {
        if (i != 0) {
            indexes_string += ", ";
        }

        indexes_string += std::to_string(sp_indexes_->operator[](i));
    }

    std::string begins_string;

    for (size_t i = 0; i < sp_begins_->size(); i++) {
        if (i != 0) {
            begins_string += ", ";
        }

        begins_string += std::to_string(sp_begins_->operator[](i));
    }

    ss << "ComplexStrippedPartition[SP mode] { indexes = [ " << indexes_string
       << " ]; begins = [ " << begins_string
       << " ] }";

    return ss.str();
}

void ComplexStrippedPartition::sp_Product(short attribute) {
    std::vector<size_t>* new_indexes = new std::vector<size_t>();
    new_indexes->reserve(data_.GetColumnCount());
    
    std::vector<size_t>* new_begins = new std::vector<size_t>();
    size_t fill_pointer = 0;

    for (size_t begin_pointer = 0; begin_pointer < sp_begins_->size() - 1; begin_pointer++) {
        size_t group_begin = sp_begins_->operator[](begin_pointer);
        size_t group_end = sp_begins_->operator[](begin_pointer + 1);

        std::vector<std::pair<int, size_t>> values(group_end - group_begin);

        for (size_t i = group_begin; i < group_end; i++) {
            size_t index = sp_indexes_->operator[](i);
            values[i - group_begin] = { data_.GetValue(index, attribute), index };
        }

        std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
            return p1.first < p2.first;
        });

        size_t group_start = 0;
        size_t i = 1;
        
        auto AddGroup = [&]() {
            size_t group_size = i - group_start;
            
            if (group_size > 1) {
                new_begins->push_back(fill_pointer);
                fill_pointer += group_size;
                
                for (size_t j = group_start; j < group_start + group_size; ++j) {
                    new_indexes->push_back(values[j].second);
                }
            }
            group_start = i;
        };

        for (; i < values.size(); ++i) {
            if (values[i - 1].first != values[i].first)
                AddGroup();
        }

        AddGroup();
    }

    new_begins->push_back(new_indexes->size());

    sp_indexes_ = std::shared_ptr<std::vector<size_t>>(new_indexes);
    sp_begins_ = std::shared_ptr<std::vector<size_t>>(new_begins);
}

bool ComplexStrippedPartition::sp_Split(short right) const {
    for (size_t begin_pointer = 0; begin_pointer <  sp_begins_->size() - 1; begin_pointer++) {
        size_t group_begin = sp_begins_->operator[](begin_pointer);
        size_t group_end = sp_begins_->operator[](begin_pointer + 1);
        
        int group_value = data_.GetValue(sp_indexes_->operator[](group_begin), right);
        
        for (size_t i = group_begin + 1; i < group_end; i++) {
            if (data_.GetValue(sp_indexes_->operator[](i), right) != group_value) {
                return true;
            }
        }
    }

    return false;
}

std::string ComplexStrippedPartition::rb_ToString() const {
    std::stringstream ss;
    std::string indexes_string;

    for (size_t i = 0; i < rb_indexes_->size(); i++) {
        if (i != 0) {
            indexes_string += ", ";
        }

        indexes_string += "(" + std::to_string(rb_indexes_->operator[](i).first) + ";" +
                          std::to_string(rb_indexes_->operator[](i).second) + ")";
    }

    std::string begins_string;

    for (size_t i = 0; i < rb_begins_->size(); i++) {
        if (i != 0) {
            begins_string += ", ";
        }

        begins_string += std::to_string(rb_begins_->operator[](i));
    }

    ss << "ComplexStrippedPartition[RB mode] { indexes = [ " << indexes_string << " ]; begins = [ "
       << begins_string << " ] }";

    return ss.str();
}

void ComplexStrippedPartition::rb_Product(short attribute) {
    std::vector<size_t>* new_begins = new std::vector<size_t>();
    std::vector<DataFrame::range_t>* new_indexes = new std::vector<DataFrame::range_t>();

    size_t curr_begin = 0;

    for (size_t begin_pointer = 0; begin_pointer < rb_begins_->size() - 1; ++begin_pointer) {
        size_t group_begin = rb_begins_->operator[](begin_pointer);
        size_t group_end = rb_begins_->operator[](begin_pointer + 1);

        std::vector<DataFrame::value_indexes_t> intersection =
            IntersectWithAttribute(attribute, group_begin, group_end - 1);

        size_t intersection_size = intersection.size();
        size_t small_ranges_count = 0;

        auto AddGroup = [&new_indexes, &new_begins, &intersection, &curr_begin, &small_ranges_count](size_t start_index,
                                                                                                     size_t end_index) {
            if (start_index == end_index) {
                DataFrame::range_t range = intersection[start_index].second;

                if (range.second == range.first) {
                    return;
                }
            }

            for (size_t i = start_index; i <= end_index; ++i) {
                DataFrame::range_t const& range = intersection[i].second;

                if (range_size(range) < MIN_MEANINGFUL_RANGE_SIZE) {
                    small_ranges_count++;
                }

                new_indexes->push_back(std::move(range));
            }

            new_begins->push_back(curr_begin);
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

        if (!should_be_converted_to_sp_
            && intersection_size > 0
            && small_ranges_count / (double)intersection_size >= SMALL_RANGES_RATIO_TO_CONVERT) {
            
            should_be_converted_to_sp_ = true;
        }
    }

    new_begins->push_back(new_indexes->size());

    rb_indexes_ = std::shared_ptr<std::vector<DataFrame::range_t>>(new_indexes);
    rb_begins_ = std::shared_ptr<std::vector<size_t>>(new_begins);
}

bool ComplexStrippedPartition::rb_Split(short right) const {
    for (size_t begin_pointer = 0; begin_pointer < rb_begins_->size() - 1; ++begin_pointer) {
        size_t group_begin = rb_begins_->operator[](begin_pointer);
        size_t group_end = rb_begins_->operator[](begin_pointer + 1);

        int group_value = data_.GetValue(rb_indexes_->operator[](group_begin).first, right);

        for (size_t i = group_begin; i < group_end; ++i) {
            DataFrame::range_t range = rb_indexes_->operator[](i);

            for (size_t j = range.first; j <= range.second; ++j) {
                if (data_.GetValue(j, right) != group_value) {
                    return true;
                }
            }
        }
    }

    return false;
}

std::vector<DataFrame::value_indexes_t> ComplexStrippedPartition::IntersectWithAttribute(
        algos::fastod::AttributeSet::size_type attribute, size_t group_start, size_t group_end) {
    std::vector<DataFrame::value_indexes_t> result;

    std::vector<DataFrame::value_indexes_t> const& attr_ranges =
        data_.GetDataRanges().at(attribute);

    for (size_t i = group_start; i <= group_end; ++i) {
        DataFrame::range_t const& range = rb_indexes_->operator[](i);

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

} // namespace algos::fastod