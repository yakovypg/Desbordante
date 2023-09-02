#pragma once

#include <string>
#include <cstddef>
#include <functional>

#include "config/equal_nulls/type.h"
#include "table/column_layout_typed_relation_data.h"

namespace algos::fastod {

using BigInt = model::details::Placeholder;

class SchemaValue {
private:
    model::TypeId type_id_;
    model::Type const* type_;
    std::byte const* value_;

    static bool is_empty_equal_empty_;
    static config::EqNullsType is_null_equal_null_;

public:
    SchemaValue() noexcept;
    SchemaValue(model::TypeId type_id, model::Type const* type, std::byte const* value) noexcept;
    SchemaValue(const SchemaValue&) = default;
    SchemaValue &operator=(const SchemaValue&) = default;
    SchemaValue(SchemaValue&&) noexcept = default;
    SchemaValue &operator=(SchemaValue&&) noexcept = default;

    model::TypeId GetTypeId() const noexcept;
    std::byte const* GetValue() const noexcept;

    std::string ToString() const;

    int AsInt() const;
    double AsDouble() const;
    BigInt AsBigInt() const;
    std::string AsString() const;

    bool IsInt() const noexcept;
    bool IsDouble() const noexcept;
    bool IsBigInt() const noexcept;
    bool IsString() const noexcept;
    bool IsEmpty() const noexcept;
    bool IsNull() const noexcept;
    bool IsNumeric() const noexcept;

    size_t hash() const noexcept {
        if (IsInt())
            return std::hash<int>()(AsInt());
        if (IsDouble())
            return std::hash<double>()(AsDouble());
        if (IsBigInt())
            return std::hash<std::string>()(AsBigInt());
        return std::hash<std::string>()(AsString());
    }

    static SchemaValue FromTypedColumnData(model::TypedColumnData const& column, std::size_t index) noexcept;

    friend bool operator==(SchemaValue const& x, SchemaValue const& y);
    friend bool operator!=(SchemaValue const& x, SchemaValue const& y);
    friend bool operator<(SchemaValue const& x, SchemaValue const& y);
    friend bool operator<=(SchemaValue const& x, SchemaValue const& y);
    friend bool operator>(SchemaValue const& x, SchemaValue const& y);
    friend bool operator>=(SchemaValue const& x, SchemaValue const& y);
};

} // namespace algos::fastod

template <>
struct std::hash<algos::fastod::SchemaValue>
{
    size_t operator()(const algos::fastod::SchemaValue& value) const {
        return value.hash();
    }
};

