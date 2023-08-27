#pragma once

#include <string>

#include "schema_value.h"
#include "operator_type.h"

namespace algos::fastod {

class Operator {
private:
    OperatorType type_;
    
public:
    explicit Operator(OperatorType type) noexcept;

    OperatorType GetType() const noexcept;
    int GetTypeAsInt() const noexcept;
    std::string ToString() const noexcept;

    Operator Reverse() const noexcept;

    bool Oppose(Operator const& other) const noexcept;
    bool Imply(Operator const& other) const noexcept;
    bool Satisfy(SchemaValue const& first, SchemaValue const& second) const noexcept;
    bool Violate(SchemaValue const& first, SchemaValue const& second) const noexcept;

    bool IsLessOrGreater() const noexcept;

    static std::vector<Operator> SupportedOperators() noexcept;

    friend bool operator==(Operator const& x, Operator const& y);
    friend bool operator!=(Operator const& x, Operator const& y);
    bool operator()(SchemaValue const& first, SchemaValue const& second);
};

} // namespace algos::fatod