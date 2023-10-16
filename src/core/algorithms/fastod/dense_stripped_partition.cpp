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

std::string DenseStrippedPartition::ToStringWithSort() const {   
    std::stringstream ss;
    size_t curr_index = 0;

    ss << "{\n";

    std::vector<EquivalenceClass> sorted_classes;
    sorted_classes.insert(sorted_classes.end(), classes_.begin(), classes_.end());

    std::sort(sorted_classes.begin(), sorted_classes.end(), [](auto const& x, auto const& y) {
        return x.GetIndexes().at(0).GetStart() < y.GetIndexes().at(0).GetStart();
    });

    for (EquivalenceClass const& eq_class : sorted_classes) {       
        ss << "  " << eq_class.ToStringWithSort();

        if (curr_index++ < sorted_classes.size() - 1) {
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

    std::vector<EquivalenceClass> sorted_classes;
    sorted_classes.insert(sorted_classes.end(), classes_.begin(), classes_.end());

    std::sort(sorted_classes.begin(), sorted_classes.end(), [](auto const& x, auto const& y) {
        std::vector<Range> x_sorted_indexes = x.GetIndexes();
        std::vector<Range> y_sorted_indexes = y.GetIndexes();

        std::sort(x_sorted_indexes.begin(), x_sorted_indexes.end(), [](Range const& x, Range const& y) {
            return x.GetStart() < y.GetStart();
        });
        std::sort(y_sorted_indexes.begin(), y_sorted_indexes.end(), [](Range const& x, Range const& y) {
            return x.GetStart() < y.GetStart();
        });
        
        return x_sorted_indexes.at(0).GetStart() < y_sorted_indexes.at(0).GetStart();
    });
    
    for (EquivalenceClass const& eq_class : sorted_classes) {
        begins.push_back(group_start);

        std::vector<Range> sorted_indexes = eq_class.GetIndexes();

        std::sort(sorted_indexes.begin(), sorted_indexes.end(), [](Range const& x, Range const& y) {
            return x.GetStart() < y.GetStart();
        });
        
        for (Range const& range : sorted_indexes) {
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
    std::vector<EquivalenceClass> attr_classes = data_.GetClasses(attribute);

    for (EquivalenceClass const& lhs : classes_) {
        for (EquivalenceClass const& rhs : attr_classes) {
            EquivalenceClass intersection = EquivalenceClass::Intersect(lhs, rhs);

            if (intersection.Size() > 1) {
                new_classes.push_back(intersection);
            }
        }
    }

    classes_ = std::move(new_classes);
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