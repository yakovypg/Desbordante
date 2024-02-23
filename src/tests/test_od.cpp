#include <filesystem>
#include <limits>
#include <tuple>
#include <utility>

#include <boost/functional/hash.hpp>
#include <gtest/gtest.h>

#include "algorithms/od/fastod/fastod.h"

namespace {

constexpr auto input_data_path = "input_data/od_norm_data/";

size_t RunFastOdOn(std::string dataset, std::string folder) {
    auto d = input_data_path / std::filesystem::path(folder + "/" + dataset);
    algos::fastod::DataFrame data = algos::fastod::DataFrame::FromCsv(d);

    algos::fastod::Fastod fastod(std::move(data), std::numeric_limits<long>::max());
    std::vector<std::string> string_ods = fastod.DiscoverAsStrings();
    std::sort(std::begin(string_ods), std::end(string_ods));
    return boost::hash<decltype(string_ods)>()(string_ods);
}

}  // namespace

using DatasetWithHashAndFolder = std::tuple<std::string, size_t, std::string>;

class TestFastOD : public ::testing::TestWithParam<DatasetWithHashAndFolder> {};

TEST_P(TestFastOD, CorrectnessTest) {
    auto const& [dataset, expected_hash, folder] = GetParam();
    size_t actual_hash = RunFastOdOn(dataset, folder);
    EXPECT_EQ(actual_hash, expected_hash);
}

INSTANTIATE_TEST_SUITE_P(
        TestFastODSuite, TestFastOD,
        ::testing::Values(
                DatasetWithHashAndFolder{"CLASSIFICATION_norm.csv", 18091212580651051245ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"OD_norm.csv", 17715088954325427783ULL, "."},
                DatasetWithHashAndFolder{"abalone_norm.csv", 16606050467380301578ULL, "metanome"},
                DatasetWithHashAndFolder{"balance-scale_norm.csv", 0ULL, "metanome"},
                DatasetWithHashAndFolder{"breast-cancer-wisconsin.csv", 16353518380015578090ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"echocardiogram_norm.csv", 326215408910846314ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"hepatitis_norm.csv", 7742226892016166457ULL, "metanome"},
                DatasetWithHashAndFolder{"hepatitis_norm_1.csv", 17523839062965376704ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"hepatitis_norm_2.csv", 6299823600881027889ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"hepatitis_norm_3.csv", 14366259447601078560ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"hepatitis_norm_4.csv", 17523839062965376704ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"hepatitis_norm_5.csv", 10213031677476394156ULL,
                                         "metanome"},
                DatasetWithHashAndFolder{"horse_10c_norm.csv", 943929269970913831ULL, "metanome"},
                DatasetWithHashAndFolder{"iris_norm.csv", 0, "metanome"},
                DatasetWithHashAndFolder{"small_2x3.csv", 14955030808612352839ULL, "."},
                DatasetWithHashAndFolder{"small_3x3.csv", 3159022758505113431ULL, "."}));
