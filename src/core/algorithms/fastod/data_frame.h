#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <type_traits>

#include "config/equal_nulls/type.h"
#include "table/column_layout_typed_relation_data.h"

namespace algos::fastod {

class DataFrame {
private:
    std::vector<std::vector<int>> data_;

    static std::vector<std::pair<const std::byte*, int>> CreateIndexedColumnData(const model::TypedColumnData& column);
    static std::vector<int> ConvertColumnDataToIntegers(const model::TypedColumnData& column);

public:
    DataFrame() = default;
    DataFrame(const DataFrame&) = delete;
    DataFrame& operator=(const DataFrame&) = delete;
    DataFrame(DataFrame&&) noexcept = default;
    DataFrame& operator=(DataFrame&&) = default;
    ~DataFrame() noexcept = default;

    explicit DataFrame(const std::vector<model::TypedColumnData>& columns_data);

    int GetValue(int tuple_index, int attribute_index) const;
    
    std::size_t GetColumnCount() const;
    std::size_t GetTupleCount() const;

    static DataFrame FromCsv(std::filesystem::path const& path);

    static DataFrame FromCsv(std::filesystem::path const& path,
                             char separator,
                             bool has_header,
                             config::EqNullsType is_null_equal_null);

};

} // namespace algos::fatod
