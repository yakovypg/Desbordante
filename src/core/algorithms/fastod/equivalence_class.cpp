#include <string>
#include <numeric>
#include <sstream>
#include <optional>
#include <queue>

#include "equivalence_class.h"

namespace algos::fastod {

EquivalenceClass::EquivalenceClass(AttributeSet::size_type attribute) noexcept
    : indexes_({}), size_(0), attribute_(attribute) {}

EquivalenceClass::EquivalenceClass(std::vector<Range> indexes, AttributeSet::size_type attribute) {  
    attribute_ = attribute;
    indexes_ = std::move(indexes);
    
    size_ = std::accumulate(indexes_.begin(), indexes_.end(), 0, [](size_t sum, Range const& range) {
        return sum + range.Size();
    });
}

size_t EquivalenceClass::Size() const noexcept {
    return size_;
}

std::vector<Range> const& EquivalenceClass::GetIndexes() const {
    return indexes_;
}

AttributeSet::size_type EquivalenceClass::GetAttribute() const {
    return attribute_;
}

std::string EquivalenceClass::ToString() const {
    std::stringstream ss;
    size_t curr_index = 0;

    ss << "{ ";
    
    for (Range const& range : indexes_) {
        ss << range.ToString();

        if (curr_index++ < indexes_.size() - 1) {
            ss << ", ";
        }
    }

    ss << " }";

    return ss.str();
}

std::string EquivalenceClass::ToStringWithSort() const {
    std::stringstream ss;
    size_t curr_index = 0;

    ss << "{ ";

    std::vector<Range> sorted_indexes;
    sorted_indexes.insert(sorted_indexes.end(), indexes_.begin(), indexes_.end());

    std::sort(sorted_indexes.begin(), sorted_indexes.end(), [](Range const& x, Range const& y) {
        return x.GetStart() < y.GetStart();
    });
    
    for (Range const& range : sorted_indexes) {
        ss << range.ToString();

        if (curr_index++ < sorted_indexes.size() - 1) {
            ss << ", ";
        }
    }

    ss << " }";

    return ss.str();
}

int EquivalenceClass::GetRepresentative(DataFrame const& data) const {
    return GetRepresentative(data, attribute_);
}

int EquivalenceClass::GetRepresentative(DataFrame const& data, algos::fastod::AttributeSet::size_type attribute) const {
    return size_ > 0
        ? data.GetValue(indexes_[0].GetStart(), attribute)
        : 0;
}

std::vector<int> EquivalenceClass::GetValues(DataFrame const& data) const {
    std::vector<int> values(size_);  
    size_t curr_index = 0;

    for (Range const& range : indexes_) {
        for (size_t index = range.GetStart(); index <= range.GetEnd(); ++index) {
            values[curr_index++] = data.GetValue(index, attribute_);
        }
    }

    return values;
}

std::vector<std::pair<int, size_t>> EquivalenceClass::GetIndexedValues(DataFrame const& data) const {
    return GetIndexedValues(data, attribute_);
}

std::vector<std::pair<int, size_t>> EquivalenceClass::GetIndexedValues(DataFrame const& data,
    algos::fastod::AttributeSet::size_type attribute) const {
    
    std::vector<std::pair<int, size_t>> values(size_); 
    size_t curr_index = 0;

    for (Range const& range : indexes_) {
        for (size_t index = range.GetStart(); index <= range.GetEnd(); ++index) {
            values[curr_index++] = std::make_pair(data.GetValue(index, attribute), index);
        }
    }

    return values;
}

/*
EquivalenceClass EquivalenceClass::Intersect(EquivalenceClass const& lhs, EquivalenceClass const& rhs) {
    if (lhs.Size() > rhs.Size()) {
        return Intersect(std::move(rhs), std::move(lhs));
    }

    std::vector<Range> res_ranges;
    std::vector<Range> const& rhs_ranges = rhs.GetIndexes();

    size_t next_rhs_range_index = 0; 

    for (Range lhs_range : lhs.GetIndexes()) {
        for (size_t i = next_rhs_range_index; i < rhs_ranges.size(); ++i) {
            Range curr_range = rhs_ranges[i];

            if (lhs_range < curr_range || lhs_range > curr_range) {
                continue;
            }
            
            // if (lhs_range < curr_range) {
            //     next_rhs_range_index = i;
            //     break;
            // }

            // if (lhs_range > curr_range) {
            //     next_rhs_range_index++;
            //     continue;
            // }

            Range intersection = Range::Intersect(lhs_range, curr_range).value();
            res_ranges.push_back(intersection);

            std::optional<Range> diff = Range::DifferenseWithLeftShift(lhs_range, intersection);

            if (diff.has_value()) {
                //next_rhs_range_index++;
                lhs_range = diff.value();
            }
            else {
                break;
            }
        }
    }

    return EquivalenceClass(res_ranges, rhs.GetAttribute());
}
*/

EquivalenceClass EquivalenceClass::Intersect(EquivalenceClass const& lhs, EquivalenceClass const& rhs) {
    if (lhs.Size() > rhs.Size()) {
        return Intersect(std::move(rhs), std::move(lhs));
    }

    std::queue<Range> lhs_ranges;
    std::vector<Range> res_ranges;
    std::vector<Range> const& rhs_ranges = rhs.GetIndexes();

    for (Range const& range : lhs.GetIndexes()) {
        lhs_ranges.emplace(range);
    }

    size_t next_rhs_range_index = 0; 

    while (!lhs_ranges.empty()) {
        Range lhs_range = lhs_ranges.front();
        lhs_ranges.pop();
        
        for (size_t i = next_rhs_range_index; i < rhs_ranges.size(); ++i) {
            Range const& rhs_range = rhs_ranges.at(i);

            if (lhs_range < rhs_range || lhs_range > rhs_range) {
                continue;
            }

            Range intersection = Range::Intersect(lhs_range, rhs_range).value();
            res_ranges.push_back(intersection);

            std::vector<Range> diff = Range::Differense(lhs_range, intersection);
            size_t diff_size = diff.size();

            if (diff_size > 0) {
                lhs_range = diff[0];

                if (diff_size > 1) {
                    lhs_ranges.emplace(diff[1]);
                }
            }
            else {
                break;
            }
        }
    }

    return EquivalenceClass(res_ranges, rhs.GetAttribute());
}

std::vector<EquivalenceClass> EquivalenceClass::ExtractClasses(std::vector<int> const& data,
                                                               algos::fastod::AttributeSet::size_type attribute) {
    std::vector<EquivalenceClass> classes_;
    std::vector<std::pair<int, size_t>> indexed_data(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        indexed_data[i] = { data[i], i };
    }

    std::sort(indexed_data.begin(), indexed_data.end(), [](auto const& x, auto const& y) {
        return x.first < y.first;
    });

    auto AddClass = [attribute, &classes_, &indexed_data](size_t start_index, size_t end_index) {
        if (end_index == start_index)
            return;

        std::vector<Range> indexes = Range::ExtractRanges(indexed_data, start_index, end_index);
        EquivalenceClass new_class(indexes, attribute);

        classes_.push_back(new_class);
    };

    size_t class_start_index = 0;

    for (size_t i = 1; i < indexed_data.size(); ++i) {
        if (indexed_data[i - 1].first == indexed_data[i].first)
            continue;
        
        AddClass(class_start_index, i - 1);
        class_start_index = i;
    }

    AddClass(class_start_index, indexed_data.size() - 1);

    return classes_;
}

} // namespace algos::fastod
