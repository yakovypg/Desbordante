#pragma once

#include <string>

#include "data/data_frame.h"

namespace algos::fastod {

struct SingleAttributePredicate {
    size_t attribute;
    bool ascending;

    SingleAttributePredicate(size_t attribute, bool ascending) noexcept;
    std::string ToString() const;

    template <typename T>
    bool Satisfy(DataFrame const& data,
                 size_t first_tuple_index,
                 size_t second_tuple_index) const noexcept {
        if (ascending)
            return data.GetValue<T>(first_tuple_index, attribute) < data.GetValue<T>(second_tuple_index, attribute);
        return data.GetValue<T>(second_tuple_index, attribute) < data.GetValue<T>(first_tuple_index, attribute);
    }

    template <typename T>
    bool Satisfy(typename constResType<T>::type first, typename constResType<T>::type second) const noexcept {
        if (ascending)
            return first < second;
        return second < first;
    }
};

bool operator==(SingleAttributePredicate const& x, SingleAttributePredicate const& y);

} // namespace algos::fatod
