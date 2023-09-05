#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <type_traits>

#include "schema_value.h"

namespace algos::fastod {

class DataFrame {
private:
    std::vector<std::vector<int>> data_;

    static std::vector<std::pair<const std::byte*, int>> CreateIndexedColumnData(const model::TypedColumnData& column);
    static std::vector<int> ConvertColumnDataToIntegers(const model::TypedColumnData& column);

public:
    DataFrame() = default;
    DataFrame(const DataFrame&) = delete;
    explicit DataFrame(const std::vector<model::TypedColumnData>& columns_data) noexcept;

    int GetValue(int tuple_index, int attribute_index) const noexcept;
    
    std::size_t GetColumnCount() const noexcept;
    std::size_t GetTupleCount() const noexcept;

    static DataFrame FromCsv(std::filesystem::path const& path);

    static DataFrame FromCsv(std::filesystem::path const& path,
                             char separator,
                             bool has_header,
                             config::EqNullsType is_null_equal_null);

};

} // namespace algos::fatod
