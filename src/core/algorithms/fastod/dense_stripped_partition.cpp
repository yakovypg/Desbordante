#include <sstream>
#include <iostream>

#include "range.h"
#include "equivalence_class.h"
#include "stripped_partition.h"
#include "dense_stripped_partition.h"

namespace algos::fastod {

DenseStrippedPartition::DenseStrippedPartition(const DataFrame& data) : data_(std::move(data)) {
    size_t tuple_count = data.GetTupleCount();
    const AttributeSet::size_type attribute = 1;
    
    classes_.push_back(EquivalenceClass({ Range(0, tuple_count - 1) }, attribute));
}

std::string DenseStrippedPartition::ToString() const {
    std::stringstream ss;
    size_t curr_index = 0;

    ss << "{\n";

    for (EquivalenceClass const& eq_class : classes_) {
        ss << "  " << eq_class.ToString();

        if (curr_index++ < classes_.size() - 1) {
            ss << ",\n";
        }
    }

    ss << "\n}";

    return ss.str();
}

StrippedPartition DenseStrippedPartition::ToStrippedPartition() const {
    std::vector<size_t> indexes;
    std::vector<size_t> begins;

    size_t group_start = 0;
    
    for (EquivalenceClass const& eq_class : classes_) {
        begins.push_back(group_start);
        
        for (Range const& range : eq_class.GetIndexes()) {
            std::vector<size_t> range_values = range.ToVector();
            indexes.insert(indexes.end(), range_values.begin(), range_values.end());

            group_start += range_values.size();
        }
    }

    begins.push_back(indexes.size());

    return StrippedPartition(data_, indexes, begins);
}

DenseStrippedPartition& DenseStrippedPartition::operator=(const DenseStrippedPartition& other) {
    if (this == &other) {
        return *this;
    }
    
    classes_ = other.classes_;
    //data_ = other.data_;

    return *this;
}

void DenseStrippedPartition::Product(short attribute) {
    std::vector<EquivalenceClass> new_classes;
    new_classes.reserve(data_.GetColumnCount());

    for (EquivalenceClass const& eq_class : classes_) {
        std::vector<std::pair<int, size_t>> values = eq_class.GetIndexedValues(data_, attribute);

        std::sort(values.begin(), values.end(), [](const auto& x, const auto& y) {
            return x.first < y.first;
        });

        auto AddClass = [attribute, &new_classes, &values](size_t start_index, size_t end_index) {
            if (end_index == start_index)
                return;

            std::vector<Range> indexes = Range::ExtractRanges(values, start_index, end_index);
            EquivalenceClass new_class(indexes, attribute);

            new_classes.push_back(new_class);
        };

        size_t class_start_index = 0;

        for (size_t i = 1; i < values.size(); ++i) {
            if (values[i - 1].first == values[i].first)
                continue;
            
            AddClass(class_start_index, i - 1);
            class_start_index = i;
        }

        AddClass(class_start_index, values.size() - 1);
    }

    classes_ = std::move(new_classes);

    //std::cout << ToStrippedPartition().ToString() << std::endl;
}

bool DenseStrippedPartition::Split(short right) {
    for (EquivalenceClass const& eq_class : classes_) {
        int representative = eq_class.GetRepresentative(data_, right);
        std::vector<Range> const& indexes = eq_class.GetIndexes();

        for (Range const& range : indexes) {
            for (size_t i = range.GetStart(); i <= range.GetEnd(); ++i) {
                if (data_.GetValue(i, right) != representative) {
                    //std::cout << "Split:\t" << "true" << std::endl;
                    return true;
                }
            }
        }
    }

    //std::cout << "Split:\t" << "false" << std::endl;
    return false;
}

} // namespace algos::fastod