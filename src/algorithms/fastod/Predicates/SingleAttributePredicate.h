#pragma once

#include <vector>
#include <string>

#include "Operator.h"
#include "DataFrame.h"

namespace algos::fastod {

class SingleAttributePredicate {
private:
    int attribute_;
    Operator operator_;
    static std::vector<std::vector<SingleAttributePredicate>> cache_;
    
public:
    SingleAttributePredicate(int attribute, Operator const& op) noexcept;

    int GetAttribute() const noexcept;
    Operator const& GetOperator() const noexcept;

    std::string ToString() const;
    int GetHashCode() const noexcept;

    bool Violate(DataFrame const& data,
                 int first_tuple_index,
                 int second_tuple_index) const noexcept;

    static SingleAttributePredicate GetInstance(int attribute, Operator const& op);
};

} // namespace algos::fatod
