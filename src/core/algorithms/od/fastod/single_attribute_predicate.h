#pragma once

#include <memory>
#include <optional>
#include <string>

#include "data_frame.h"

namespace algos::fastod {

struct SingleAttributePredicate {
    size_t attribute;
    bool ascending;

    SingleAttributePredicate(size_t attribute, bool ascending);
    std::string ToString() const;

    bool Satisfy(std::shared_ptr<DataFrame> data, size_t first_tuple_index,
                 size_t second_tuple_index) const {
        if (ascending)
            return data->GetValue(first_tuple_index, attribute) <
                   data->GetValue(second_tuple_index, attribute);
        return data->GetValue(second_tuple_index, attribute) <
               data->GetValue(first_tuple_index, attribute);
    }

    bool Satisfy(int first, int second) const {
        if (ascending) return first < second;
        return second < first;
    }

    size_t hash() const;
};

bool operator==(SingleAttributePredicate const& x, SingleAttributePredicate const& y);

}  // namespace algos::fastod
