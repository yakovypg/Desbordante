#pragma once

#include <string>
#include "schema_value.h"

namespace algos::fastod {

// CHANGE: rename from DataAndIndex
// REASON: column types
class ValuePair {
private:
    std::pair<SchemaValue, SchemaValue> pair_;
    
public:
    ValuePair(SchemaValue const& data, SchemaValue const& index) noexcept;

    const SchemaValue& GetFirst() const noexcept;
    const SchemaValue& GetSecond() const noexcept;
    SchemaValue& GetFirst() noexcept;
    SchemaValue& GetSecond() noexcept;

    std::string ToString() const noexcept;
    friend bool operator<(const ValuePair& x, const ValuePair& y);
    friend bool operator>(const ValuePair& x, const ValuePair& y);
};

} //namespace algos::fastod
