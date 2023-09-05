#include <utility>
#include <filesystem>

#include "csv_parser/csv_parser.h"

#include "data_frame.h"
#include "../attribute_set.h"

using namespace algos::fastod;

#include <iostream>

DataFrame::DataFrame(const std::vector<model::TypedColumnData>& columns_data) noexcept {
    std::size_t cols_num = columns_data.size();  
    assert(cols_num != 0);

    std::transform(columns_data.cbegin(), columns_data.cend(),
        std::back_inserter(data_), ConvertColumnDataToIntegers);

    // data_ = std::vector<std::vector<int>>(cols_num);

    // for (std::size_t i = 0; i < cols_num; ++i) {
    //     auto v = ConvertColumnDataToIntegers(columns_data[i]);
    //     data_[i] = v;
    // }

    // std::cout << "\nTEST START\n\n";

    // std::vector<std::string> vect(data_.size());

    // for (std::size_t i = 0; i < data_.size(); ++i) {
    //     for (std::size_t j = 0; j < data_[0].size(); ++j) {
    //         vect[j] += std::to_string(data_[i][j]);

    //         if (i < data_.size() - 1)
    //             vect[j] += ",";
    //         else
    //             vect[j] += '\n';
    //     }
    // }

    // for (std::size_t i = 0; i < vect.size(); ++i) {
    //     std::cout << vect[i];
    // }

    // std::cout << "\nTEST END\n";

    ASIterator::MAX_COLS = cols_num;
}

int DataFrame::GetValue(int tuple_index, int attribute_index) const noexcept {
    return data_[attribute_index][tuple_index];
}

std::size_t DataFrame::GetColumnCount() const noexcept {
    return data_.size();
}

std::size_t DataFrame::GetTupleCount() const noexcept {
    return data_.size() > 0
        ? data_.at(0).size()
        : 0;
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

std::vector<std::pair<const std::byte*, int>> DataFrame::CreateIndexedColumnData(const model::TypedColumnData& column) {
    std::vector<const std::byte*> data = column.GetData();
    std::vector<std::pair<const std::byte*, int>> indexed_column_data(data.size());

    for (std::size_t i = 0; i < data.size(); ++i) {
        indexed_column_data[i] = std::make_pair(data[i], i);
    }

    return std::move(indexed_column_data);
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

    return std::move(converted_column);
}
