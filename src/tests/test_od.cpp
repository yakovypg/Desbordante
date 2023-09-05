#include <limits>
#include <utility>

#include <gtest/gtest.h>
#include <boost/functional/hash.hpp>

#include "algorithms/fastod/fastod.h"

namespace {

constexpr auto input_data_path = "input_data/od_norm_data";

size_t RunFastOdOn(std::string dataset) {
    auto d = input_data_path / std::filesystem::path(dataset);
    algos::fastod::DataFrame data =
        algos::fastod::DataFrame::FromCsv(d);

    algos::fastod::Fastod fastod(std::move(data), std::numeric_limits<long>::max());
    std::vector<algos::fastod::CanonicalOD> ods = fastod.Discover();
    std::vector<std::string> string_ods;
    string_ods.reserve(ods.size());
    std::transform(std::begin(ods), std::end(ods), std::back_inserter(string_ods),
                   [](auto const& od) { return od.ToString(); });
    std::sort(std::begin(string_ods), std::end(string_ods));
    return boost::hash<decltype(string_ods)>()(string_ods);
}

}  // namespace

using DatasetWithHash = std::pair<std::string, size_t>;

class TestFastOD : public ::testing::TestWithParam<DatasetWithHash> {};

TEST_P(TestFastOD, CorrectnessTest) {
    auto const& [dataset, expected_hash] = GetParam();
    size_t actual_hash = RunFastOdOn(dataset);
    EXPECT_EQ(actual_hash, expected_hash);
}

INSTANTIATE_TEST_SUITE_P(TestFastODSuite, TestFastOD,
    ::testing::Values(
            DatasetWithHash{"CLASSIFICATION_norm.csv", 18091212580651051245ULL},
            DatasetWithHash{"OD_norm.csv", 17715088954325427783ULL},
            DatasetWithHash{"abalone_norm.csv", 16606050467380301578ULL},
            DatasetWithHash{"balance-scale_norm.csv", 0ULL},
            DatasetWithHash{"breast-cancer-wisconsin.csv", 16353518380015578090ULL},
            DatasetWithHash{"echocardiogram_norm.csv", 326215408910846314ULL},
            DatasetWithHash{"hepatitis_norm.csv", 7742226892016166457ULL},
            DatasetWithHash{"hepatitis_norm_1.csv", 17523839062965376704ULL},
            DatasetWithHash{"hepatitis_norm_2.csv", 6299823600881027889ULL},
            DatasetWithHash{"hepatitis_norm_3.csv", 14366259447601078560ULL},
            DatasetWithHash{"hepatitis_norm_4.csv", 17523839062965376704ULL},
            DatasetWithHash{"hepatitis_norm_5.csv", 10213031677476394156ULL},
            DatasetWithHash{"horse_10c_norm.csv", 943929269970913831ULL},
            DatasetWithHash{"iris_norm.csv", 0},
            DatasetWithHash{"small_2x3.csv", 14955030808612352839ULL},
            DatasetWithHash{"small_3x3.csv", 3159022758505113431ULL}
    )
);

