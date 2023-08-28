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
    template <typename T>
    bool Satisfy(T const& first, T const& second) const noexcept {
        return !Violate<T>(first, second);
    }

    template <typename T>
    bool Violate(T const& first, T const& second) const noexcept {
        switch (type_) {
            case OperatorType::Equal: return first != second;
            case OperatorType::Less: return first >= second;
            case OperatorType::Greater: return first <= second;
            case OperatorType::LessOrEqual: return first > second;
            case OperatorType::GreaterOrEqual: return first < second;
            case OperatorType::NotEqual: return first == second;

            default: return false;
        }
    }

    bool IsLessOrGreater() const noexcept;

    static std::vector<Operator> SupportedOperators() noexcept;

    friend bool operator==(Operator const& x, Operator const& y);
    friend bool operator!=(Operator const& x, Operator const& y);
    template <typename T>
    bool operator()(T const& first, T const& second) {
        return Satisfy<T>(first, second);
    }
};

} // namespace algos::fatod
