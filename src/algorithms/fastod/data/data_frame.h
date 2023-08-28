#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <type_traits>

#include "util/config/equal_nulls/type.h"
#include "model/column_layout_typed_relation_data.h"

#include "schema_value.h"

namespace algos::fastod {

template <typename T>
struct resType
{ using type = std::conditional_t<std::is_same_v<T, std::string>, T&, T>; };

template <typename T>
struct constResType
{ using type = std::conditional_t<std::is_same_v<T, std::string>, const T&, T>; };

class DataFrame {
private:
    std::vector<model::TypedColumnData> columns_data_;
    std::vector<std::vector<SchemaValue>> data_;
    std::vector<std::vector<int>> dataInt;
    std::vector<std::vector<double>> dataDouble;
    std::vector<std::vector<std::string>> dataString;

public:
    DataFrame() = default;
    DataFrame(const DataFrame&) = delete;
    explicit DataFrame(std::vector<model::TypedColumnData> columns_data) noexcept;

    // const SchemaValue& GetValue(int tuple_index, int attribute_index) const noexcept;
    // SchemaValue& GetValue(int tuple_index, int attribute_index) noexcept;
    
    template <typename T>
    typename constResType<T>::type GetValue(size_t tuple_index, size_t attr_index) const noexcept {
        // std::cout << "tInd = " << tuple_index << " aInt = " << attr_index << " size = " << dataInt[attr_index].size() << std::endl;
        if constexpr (std::is_same_v<T, int>)
            return dataInt[attr_index][tuple_index];
        else if constexpr (std::is_same_v<T, double>)
            return dataDouble[attr_index][tuple_index];
        else
            return dataString[attr_index][tuple_index];
    }

    template <typename T>
    typename resType<T>::type GetValue(size_t tuple_index, size_t attr_index) noexcept {
        if constexpr (std::is_same<T, int>::value)
            return dataInt[attr_index][tuple_index];
        else if constexpr (std::is_same<T, double>::value)
            return dataDouble[attr_index][tuple_index];
        else
            return dataString[attr_index][tuple_index];
    }
    
    std::size_t GetColumnCount() const noexcept;
    std::size_t GetTupleCount() const noexcept;
    model::TypeId GetColTypeId(size_t col) const noexcept;

    static DataFrame FromCsv(std::filesystem::path const& path);

    static DataFrame FromCsv(std::filesystem::path const& path,
                             char separator,
                             bool has_header,
                             util::config::EqNullsType is_null_equal_null);

};

} // namespace algos::fatod
