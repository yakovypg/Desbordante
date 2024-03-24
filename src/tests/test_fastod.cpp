#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "algorithms/algo_factory.h"
#include "algorithms/od/fastod/fastod.h"
#include "algorithms/od/fastod/hashing/hashing.h"
#include "all_csv_configs.h"
#include "config/names.h"
#include "config/time_limit/type.h"
#include "csv_config_util.h"

namespace tests {

namespace {

size_t RunFastod(CSVConfig const& csv_config) {
    using namespace config::names;

    algos::StdParamsMap params{{kCsvConfig, csv_config}};

    std::unique_ptr<algos::Fastod> fastod = algos::CreateAndLoadAlgorithm<algos::Fastod>(params);

    std::vector<std::string> string_ods = fastod->DiscoverAsStrings();
    std::sort(std::begin(string_ods), std::end(string_ods));

    std::size_t result_hash = 0;

    for (std::string const& od : string_ods) {
        size_t od_hash = std::hash<std::string>{}(od);
        result_hash = algos::fastod::hashing::CombineHashes(result_hash, od_hash);
    }

    return result_hash;
}

}  // namespace

class FastodResultHashTest : public ::testing::TestWithParam<CSVConfigHash> {};

TEST_P(FastodResultHashTest, CorrectnessTest) {
    CSVConfigHash csv_config_hash = GetParam();
    size_t actual_hash = RunFastod(csv_config_hash.config);
    EXPECT_EQ(actual_hash, csv_config_hash.hash);
}

INSTANTIATE_TEST_SUITE_P(
        TestFastodSuite, FastodResultHashTest,
        ::testing::Values(CSVConfigHash{kOdTestNormOd, 11980520805314995804UL},
                          CSVConfigHash{kOdTestNormSmall2x3, 7457915278574020764UL},
                          CSVConfigHash{kOdTestNormSmall3x3, 3318291924553133612UL},
                          CSVConfigHash{kOdTestNormAbalone, 8494646055080399391UL},
                          CSVConfigHash{kOdTestNormBalanceScale, 0ULL},
                          CSVConfigHash{kOdTestNormBreastCancerWisconsin, 16845062592796597733UL},
                          CSVConfigHash{kOdTestNormEchocardiogram, 2811588447932787109UL},
                          CSVConfigHash{kOdTestNormHepatitis1, 132585063305091933UL},
                          CSVConfigHash{kOdTestNormHepatitis2, 10199178000978455890UL},
                          CSVConfigHash{kOdTestNormHepatitis3, 3063999440011758644UL},
                          CSVConfigHash{kOdTestNormHepatitis4, 132585063305091933UL},
                          CSVConfigHash{kOdTestNormHepatitis5, 11668977472753401458UL},
                          CSVConfigHash{kOdTestNormHepatitis, 8347405483583260580UL},
                          CSVConfigHash{kOdTestNormHorse10c, 13235589009124491858UL},
                          CSVConfigHash{kOdTestNormIris, 0UL},
                          CSVConfigHash{kBernoulliRelation, 3072345414994597861UL},
                          CSVConfigHash{kTestFD, 10356217265356097778UL},
                          CSVConfigHash{kWDC_astrology, 16412680650821272398UL},
                          CSVConfigHash{kWDC_game, 17253963663585814974UL},
                          CSVConfigHash{kWDC_planetz, 16723826645866677286UL},
                          CSVConfigHash{kWDC_symbols, 4539870310106546587UL}));

}  // namespace tests
