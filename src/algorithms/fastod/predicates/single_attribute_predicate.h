#pragma once

#include <vector>
#include <string>

#include "operator.h"
#include "data_frame.h"

namespace algos::fastod {

class SingleAttributePredicate {
private:
    size_t attribute_;
    Operator operator_;
    static std::vector<std::vector<SingleAttributePredicate>> cache_;
    
public:
    SingleAttributePredicate(size_t attribute, Operator const& op) noexcept;

    size_t GetAttribute() const noexcept;
    Operator const& GetOperator() const noexcept;

    std::string ToString() const;
    size_t GetHashCode() const noexcept;

    bool Violate(DataFrame const& data,
                 size_t first_tuple_index,
                 size_t second_tuple_index) const noexcept;

    static SingleAttributePredicate GetInstance(size_t attribute, Operator const& op);

    friend bool operator==(SingleAttributePredicate const& x, SingleAttributePredicate const& y);
};

} // namespace algos::fatod
