#pragma once

#include <vector>
#include <string>
#include <optional>
#include <filesystem>
#include <type_traits>

#include "config/equal_nulls/type.h"
#include "table/column_layout_typed_relation_data.h"

#include "attribute_set.h"

namespace algos::fastod {

class DataFrame {
public:
    using range_t = std::pair<size_t, size_t>;
    using value_indexes_t = std::pair<int, range_t>;

private:
    std::vector<std::vector<int>> data_;
    std::vector<std::vector<DataFrame::value_indexes_t>> data_ranges_;
    std::vector<std::vector<size_t>> range_item_placement_;

    AttributeSet attrs_with_ranges_;

    void RecognizeAttributesWithRanges();

    static std::vector<std::pair<const std::byte*, int>> CreateIndexedColumnData(const model::TypedColumnData& column);
    static std::vector<int> ConvertColumnDataToIntegers(const model::TypedColumnData& column);
    static std::vector<DataFrame::value_indexes_t> ExtractRangesFromColumn(std::vector<int> const& column);
    static std::optional<size_t> FindRangeIndexByItem(size_t item, std::vector<DataFrame::value_indexes_t> const& ranges);

public:
    DataFrame(const DataFrame&) = delete;
    DataFrame& operator=(const DataFrame&) = delete;

    DataFrame() = default;
    DataFrame(DataFrame&&) noexcept = default;
    DataFrame& operator=(DataFrame&&) = default;
    ~DataFrame() noexcept = default;

    explicit DataFrame(const std::vector<model::TypedColumnData>& columns_data);

    int GetValue(int tuple_index, AttributeSet::size_type attribute_index) const;
    std::vector<std::vector<DataFrame::value_indexes_t>> const& GetDataRanges() const;
    size_t GetRangeIndexByItem(size_t item, AttributeSet::size_type attribute) const;
    AttributeSet const& GetAttributesWithRanges() const;
    
    AttributeSet::size_type GetColumnCount() const;
    std::size_t GetTupleCount() const;

    bool IsAttributesMostlyRangeBased(AttributeSet attributes) const;

    static DataFrame FromCsv(std::filesystem::path const& path);

    static DataFrame FromCsv(std::filesystem::path const& path,
                             char separator,
                             bool has_header,
                             config::EqNullsType is_null_equal_null);
};

} // namespace algos::fatod

namespace algos::fastod {

inline size_t range_size(DataFrame::range_t const& range) {
    return range.second - range.first + 1;
}

} // namespace algos::fatod
