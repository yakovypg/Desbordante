#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <type_traits>

#include "config/equal_nulls/type.h"
#include "table/column_layout_typed_relation_data.h"

#include "attribute_set.h"
#include "equivalence_class.h"

namespace algos::fastod {

class EquivalenceClass;

class DataFrame {
private:
    std::vector<std::vector<int>> data_;
    std::vector<std::vector<EquivalenceClass>> classes_;

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

    int GetValue(int tuple_index, AttributeSet::size_type attribute_index) const;
    std::vector<EquivalenceClass> const& GetClasses(AttributeSet::size_type attribute_index) const;
    
    AttributeSet::size_type GetColumnCount() const;
    std::size_t GetTupleCount() const;

    static DataFrame FromCsv(std::filesystem::path const& path);

    static DataFrame FromCsv(std::filesystem::path const& path,
                             char separator,
                             bool has_header,
                             config::EqNullsType is_null_equal_null);

};

} // namespace algos::fatod
