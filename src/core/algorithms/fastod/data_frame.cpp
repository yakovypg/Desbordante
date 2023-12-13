#include <utility>
#include <filesystem>
#include <iostream>
#include <optional>
#include <assert.h>

#include "csv_parser/csv_parser.h"

#include "data_frame.h"

using namespace algos::fastod;

DataFrame::DataFrame(const std::vector<model::TypedColumnData>& columns_data) {
    AttributeSet::size_type cols_num = columns_data.size();  
    assert(cols_num != 0);

    data_.reserve(cols_num);
    data_ranges_.reserve(cols_num);
    range_item_placement_.reserve(cols_num);

    std::transform(columns_data.cbegin(), columns_data.cend(),
        std::back_inserter(data_), ConvertColumnDataToIntegers);

    std::transform(data_.cbegin(), data_.cend(),
        std::back_inserter(data_ranges_), ExtractRangesFromColumn);
    
    for (size_t column = 0; column < cols_num; ++column) {
        size_t tuple_count = data_.at(column).size();

        std::vector<size_t> curr_column_item_placement;
        curr_column_item_placement.reserve(tuple_count);
        
        for (size_t i = 0; i < tuple_count; ++i) {
            std::optional<size_t> range_index = FindRangeIndexByItem(i, data_ranges_[column]);            
            assert(range_index.has_value());

            curr_column_item_placement.push_back(range_index.value());
        }

        range_item_placement_.push_back(std::move(curr_column_item_placement));
    }

    RecognizeAttributesWithRanges();
}

int DataFrame::GetValue(int tuple_index, AttributeSet::size_type attribute_index) const {
    return data_[attribute_index][tuple_index];
}

std::vector<std::vector<DataFrame::value_indexes_t>> const& DataFrame::GetDataRanges() const {
    return data_ranges_;
}

size_t DataFrame::GetRangeIndexByItem(size_t item, AttributeSet::size_type attribute) const {
    return range_item_placement_.at(attribute).at(item);
}

AttributeSet const& DataFrame::GetAttributesWithRanges() const {
    return attrs_with_ranges_;
}

AttributeSet::size_type DataFrame::GetColumnCount() const {
    return data_.size();
}

std::size_t DataFrame::GetTupleCount() const {
    return data_.size() > 0
        ? data_.at(0).size()
        : 0;
}

bool DataFrame::IsAttributesMostlyRangeBased(AttributeSet attributes) const {
    AttributeSet remaining_attrs = intersect(attrs_with_ranges_, attributes);

    AttributeSet::size_type attrs_count = attributes.count();
    AttributeSet::size_type remaining_attrs_count = remaining_attrs.count();

    const double accept_range_based_partition_factor = 0.5;

    return (double)remaining_attrs_count / attrs_count >= accept_range_based_partition_factor;
}

DataFrame DataFrame::FromCsv(std::filesystem::path const& path) {
    return FromCsv(path, ',', true, true);
}

DataFrame DataFrame::FromCsv(std::filesystem::path const& path,
                             char separator,
                             bool has_header,
                             config::EqNullsType is_null_equal_null) {
    CSVParser parser = CSVParser(path, separator, has_header);
    std::vector<model::TypedColumnData> columns_data = model::CreateTypedColumnData(parser, is_null_equal_null);

    return DataFrame(std::move(columns_data));
}

void DataFrame::RecognizeAttributesWithRanges() {
    const double accept_factor = 0.001;
    
    for (size_t i = 0; i < data_ranges_.size(); ++i) {
        size_t items_count = data_[i].size();
        size_t ranges_count = data_ranges_[i].size();

        if ((double)ranges_count / items_count >= accept_factor) {
            attrs_with_ranges_.set(i, true);
        }
    }
}

std::vector<std::pair<const std::byte*, int>> DataFrame::CreateIndexedColumnData(const model::TypedColumnData& column) {
    std::vector<const std::byte*> data = column.GetData();
    std::vector<std::pair<const std::byte*, int>> indexed_column_data(data.size());

    for (std::size_t i = 0; i < data.size(); ++i) {
        indexed_column_data[i] = std::make_pair(data[i], i);
    }

    return indexed_column_data;
}

std::vector<int> DataFrame::ConvertColumnDataToIntegers(const model::TypedColumnData& column) {
    std::vector<std::pair<const std::byte*, int>> indexed_column_data = CreateIndexedColumnData(column);

    auto less = [&column](std::pair<const std::byte*, int> l, std::pair<const std::byte*, int> r) {
        if (column.IsMixed()) {
            const model::MixedType* mixed_type = column.GetIfMixed();
            return mixed_type->ValueToString(l.first) < mixed_type->ValueToString(r.first);
        }

        const model::Type& type = column.GetType();
        return type.Compare(l.first, r.first) == model::CompareResult::kLess;
    };

    auto equal = [&column](std::pair<const std::byte*, int> l, std::pair<const std::byte*, int> r) {
        if (column.IsMixed()) {
            const model::MixedType* mixed_type = column.GetIfMixed();
            return mixed_type->ValueToString(l.first) == mixed_type->ValueToString(r.first);
        }

        const model::Type& type = column.GetType();
        return type.Compare(l.first, r.first) == model::CompareResult::kEqual;
    };

    std::sort(indexed_column_data.begin(), indexed_column_data.end(), less);
    std::vector<int> converted_column(indexed_column_data.size());

    int current_value = 0;
    converted_column[indexed_column_data[0].second] = current_value;
    
    for (std::size_t i = 1; i < indexed_column_data.size(); ++i) {
        const std::pair<const std::byte*, int>& prev = indexed_column_data[i - 1];
        const std::pair<const std::byte*, int>& curr = indexed_column_data[i];

        converted_column[curr.second] = equal(prev, curr)
            ? current_value
            : ++current_value;
    }

    return converted_column;
}

std::vector<DataFrame::value_indexes_t> DataFrame::ExtractRangesFromColumn(std::vector<int> const& column) {
    std::vector<value_indexes_t> ranges;

    size_t start = 0;

    for (size_t i = 1; i < column.size(); ++i) {
        int curr_value = column[i];
        int prev_value = column[i - 1];
        
        if (curr_value != prev_value) {
            ranges.push_back({
                prev_value,
                { start, i - 1 }
            });

            start = i;
        }
    }

    ranges.push_back({
        column[column.size() - 1],
        { start, column.size() - 1 }
    });

    return ranges;
}

std::optional<size_t> DataFrame::FindRangeIndexByItem(size_t item, std::vector<DataFrame::value_indexes_t> const& ranges) {   
    auto iter = std::find_if(ranges.cbegin(), ranges.cend(), [item](auto const& p) {
        range_t const& range = p.second;
        return item >= range.first && item <= range.second;
    });

    return iter != ranges.cend()
        ? std::optional<size_t> { iter - ranges.cbegin() }
        : std::nullopt;
}
