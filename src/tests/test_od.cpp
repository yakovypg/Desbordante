#include <filesystem>
#include <limits>
#include <tuple>
#include <utility>

#include <boost/functional/hash.hpp>
#include <gtest/gtest.h>

#include "algorithms/od/fastod/fastod.h"

namespace {

namespace fs = std::filesystem;
constexpr auto input_data_path = "input_data/od_norm_data/";

size_t RunFastod(std::string const& dataset_name, std::string const& subfolder) {
    fs::path dataset_path = fs::path(input_data_path) / subfolder / dataset_name;

    algos::fastod::DataFrame data = algos::fastod::DataFrame::FromCsv(dataset_path);
    algos::fastod::Fastod fastod(std::move(data), std::numeric_limits<long>::max());

    std::vector<std::string> string_ods = fastod.DiscoverAsStrings();
    std::sort(std::begin(string_ods), std::end(string_ods));

    std::size_t result_hash = 0;

    for (std::string const& od : string_ods) {
        size_t od_hash = std::hash<std::string>{}(od);
        boost::hash_combine(result_hash, od_hash);
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
                DatasetPathAndHash{"metanome", "abalone_norm.csv", 14591019243917482774ULL},
                DatasetPathAndHash{"metanome", "balance-scale_norm.csv", 0ULL},
                DatasetPathAndHash{"metanome", "breast-cancer-wisconsin.csv",
                                   14755061899855898930ULL},
                DatasetPathAndHash{"metanome", "CLASSIFICATION_norm.csv", 14893168815353134583ULL},
                DatasetPathAndHash{"metanome", "echocardiogram_norm.csv", 158822294474149028ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_1.csv", 14388080417062769138ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_2.csv", 4325460878791588226ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_3.csv", 15687519739058379771ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_4.csv", 14388080417062769138ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm_5.csv", 1324052777825770237ULL},
                DatasetPathAndHash{"metanome", "hepatitis_norm.csv", 14839749394849763630ULL},
                DatasetPathAndHash{"metanome", "horse_10c_norm.csv", 16633700195346153616ULL},
                DatasetPathAndHash{"metanome", "iris_norm.csv", 0ULL},
                DatasetPathAndHash{"rand", "rand_30000x30.csv", 16311507802607191318ULL},
                DatasetPathAndHash{"rand", "rand_60000x30.csv", 14912818095902024302ULL},
                DatasetPathAndHash{"rand", "rand_120000x30.csv", 17702107539916091697ULL},
                DatasetPathAndHash{".", "OD_norm.csv", 17439211062045545527ULL},
                DatasetPathAndHash{".", "small_2x3.csv", 12135977562528923031ULL},
                DatasetPathAndHash{".", "small_3x3.csv", 4292031651922569007ULL}));
