#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "util/config/equal_nulls/type.h"
#include "model/column_layout_typed_relation_data.h"

#include "schema_value.h"

namespace algos::fastod {

class DataFrame {
private:
    std::vector<model::TypedColumnData> columns_data_;
    std::vector<std::vector<SchemaValue>> data_;

public:
    DataFrame() = default;
    DataFrame(const DataFrame&) = delete;
    explicit DataFrame(std::vector<model::TypedColumnData> columns_data) noexcept;

    const SchemaValue& GetValue(int tuple_index, int attribute_index) const noexcept;
    SchemaValue& GetValue(int tuple_index, int attribute_index) noexcept;
    
    std::size_t GetColumnCount() const noexcept;
    std::size_t GetTupleCount() const noexcept;

    static DataFrame FromCsv(std::filesystem::path const& path);

    static DataFrame FromCsv(std::filesystem::path const& path,
                             char separator,
                             bool has_header,
                             util::config::EqNullsType is_null_equal_null);

};

} // namespace algos::fatod
