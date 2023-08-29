#pragma once

#include <string>

#include "data_frame.h"

namespace algos::fastod {

class SingleAttributePredicate {
private:
    size_t attribute_;
    bool ascending_;
    
public:
    SingleAttributePredicate(size_t attribute, bool ascending) noexcept;

    size_t GetAttribute() const noexcept;
    bool GetAsc() const noexcept;

    std::string ToString() const;

    template <typename T>
    bool Satisfy(DataFrame const& data,
                 size_t first_tuple_index,
                 size_t second_tuple_index) const noexcept {
        if (ascending_)
            return data.GetValue<T>(first_tuple_index, attribute_) < data.GetValue<T>(second_tuple_index, attribute_);
        return data.GetValue<T>(second_tuple_index, attribute_) < data.GetValue<T>(first_tuple_index, attribute_);
    }

    template <typename T>
    bool Satisfy(typename constResType<T>::type first, typename constResType<T>::type second) const noexcept {
        if (ascending_)
            return first < second;
        return second < first;
    }

    friend bool operator==(SingleAttributePredicate const& x, SingleAttributePredicate const& y);
};

} // namespace algos::fatod
