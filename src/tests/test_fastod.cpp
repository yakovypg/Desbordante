#include <filesystem>
#include <limits>
#include <tuple>
#include <utility>

#include <gtest/gtest.h>

#include "algorithms/od/fastod/fastod.h"

namespace {

namespace fs = std::filesystem;
constexpr auto input_data_path = "input_data/od_norm_data/";

inline size_t combine_hashes(size_t first, size_t second) {
    size_t wave = second + 2654435769UL + (first << 6) + (first >> 2);
    return first ^ wave;
}

size_t RunFastod(std::string const& dataset_name, std::string const& subfolder) {
    fs::path dataset_path = fs::path(input_data_path) / subfolder / dataset_name;

    algos::fastod::DataFrame data = algos::fastod::DataFrame::FromCsv(dataset_path);
    algos::fastod::Fastod fastod(std::move(data), std::numeric_limits<long>::max());

    std::vector<std::string> string_ods = fastod.DiscoverAsStrings();
    std::sort(std::begin(string_ods), std::end(string_ods));

    std::size_t result_hash = 0;

    for (std::string const& od : string_ods) {
        size_t od_hash = std::hash<std::string>{}(od);
        result_hash = combine_hashes(result_hash, od_hash);
    }

    return result_hash;
}

}  // namespace

using DatasetPathAndHash = std::tuple<std::string, std::string, size_t>;

class TestFastod : public ::testing::TestWithParam<DatasetPathAndHash> {};

TEST_P(TestFastod, CorrectnessTest) {
    auto const& [subfolder, dataset, expected_hash] = GetParam();
    size_t actual_hash = RunFastod(dataset, subfolder);
    EXPECT_EQ(actual_hash, expected_hash);
}

INSTANTIATE_TEST_SUITE_P(
        TestFastodSuite, TestFastod,
        ::testing::Values(
                DatasetPathAndHash{"metanome", "abalone_norm.csv", 8494646055080399391ULL},
                DatasetPathAndHash{"metanome", "balance-scale_norm.csv", 0ULL},
                DatasetPathAndHash{"metanome", "breast-cancer-wisconsin.csv",
                                   16845062592796597733ULL},
                DatasetPathAndHash{"metanome", "CLASSIFICATION_norm.csv", 10775947267660160689ULL},
                DatasetPathAndHash{"metanome", "echocardiogram_norm.csv", 2811588447932787109ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_1.csv", 132585063305091933ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_2.csv", 10199178000978455890ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_3.csv", 3063999440011758644ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_4.csv", 132585063305091933ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_5.csv", 11668977472753401458ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm.csv", 8347405483583260580ULL},
                DatasetPathAndHash{"metanome", "horse_10c_norm.csv", 13235589009124491858ULL},
                DatasetPathAndHash{"metanome", "iris_norm.csv", 0ULL},
                DatasetPathAndHash{".", "OD_norm.csv", 11980520805314995804ULL},
                DatasetPathAndHash{".", "small_2x3.csv", 7457915278574020764ULL},
                DatasetPathAndHash{".", "small_3x3.csv", 3318291924553133612ULL}));
