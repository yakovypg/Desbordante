#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "all_csv_configs.h"
#include "csv_config_util.h"
#include "algorithms/algo_factory.h"
#include "algorithms/od/fastod/canonical_od.h"
#include "algorithms/od/fastod/fastod.h"
#include "config/names.h"
#include "config/time_limit/type.h"

namespace tests {

inline size_t combine_hashes(size_t first, size_t second) {
    size_t wave = second + 2654435769UL + (first << 6) + (first >> 2);
    return first ^ wave;
}

size_t RunFastod(CSVConfig const& csv_config) {
    config::InputTable input_table = MakeInputTable(csv_config);
    
    algos::fastod::DataFrame data = algos::fastod::DataFrame::FromInputTable(input_table);
    algos::fastod::Fastod fastod(std::move(data));

    std::vector<std::string> string_ods = fastod.DiscoverAsStrings();
    std::sort(std::begin(string_ods), std::end(string_ods));

    std::size_t result_hash = 0;

    for (std::string const& od : string_ods) {
        size_t od_hash = std::hash<std::string>{}(od);
        result_hash = combine_hashes(result_hash, od_hash);
    }

    return result_hash;
}

class FastodTest : public ::testing::Test {
protected:
    static std::unique_ptr<algos::fastod::Fastod> CreateAlgorithmInstance(
            config::InputTable table, config::TimeLimitSecondsType time_limit_seconds = 0u) {
        using namespace config::names;

        algos::StdParamsMap params{{kTable, table}, {kTimeLimitSeconds, time_limit_seconds}};

        return algos::CreateAndLoadAlgorithm<algos::fastod::Fastod>(params);
    }

    static void TestFastod(CSVConfig const& csv_config,
                           std::vector<std::string> expected_asc_ods_str,
                           std::vector<std::string> expected_desc_ods_str,
                           std::vector<std::string> expected_simple_ods_str) {
        using namespace algos::fastod;

        config::InputTable input_table = MakeInputTable(csv_config);

        auto algorithm = CreateAlgorithmInstance(input_table);
        algorithm->Execute();

        std::vector<AscCanonicalOD> asc_ods = algorithm->GetAscendingDependencies();
        std::vector<DescCanonicalOD> desc_ods = algorithm->GetDescendingDependencies();
        std::vector<SimpleCanonicalOD> simple_ods = algorithm->GetSimpleDependencies();

        std::vector<std::string> asc_ods_str(asc_ods.size());
        std::vector<std::string> desc_ods_str(desc_ods.size());
        std::vector<std::string> simple_ods_str(simple_ods.size());

        std::transform(asc_ods.cbegin(), asc_ods.cend(), asc_ods_str.begin(),
                       [](AscCanonicalOD const& od) { return od.ToString(); });
        std::transform(desc_ods.cbegin(), desc_ods.cend(), desc_ods_str.begin(),
                       [](DescCanonicalOD const& od) { return od.ToString(); });
        std::transform(simple_ods.cbegin(), simple_ods.cend(), simple_ods_str.begin(),
                       [](SimpleCanonicalOD const& od) { return od.ToString(); });

        ASSERT_EQ(asc_ods_str.size(), expected_asc_ods_str.size());
        ASSERT_EQ(desc_ods_str.size(), expected_desc_ods_str.size());
        ASSERT_EQ(simple_ods_str.size(), expected_simple_ods_str.size());

        std::sort(asc_ods_str.begin(), asc_ods_str.end());
        std::sort(desc_ods_str.begin(), desc_ods_str.end());
        std::sort(simple_ods_str.begin(), simple_ods_str.end());
        std::sort(expected_asc_ods_str.begin(), expected_asc_ods_str.end());
        std::sort(expected_desc_ods_str.begin(), expected_desc_ods_str.end());
        std::sort(expected_simple_ods_str.begin(), expected_simple_ods_str.end());

        for (size_t i = 0; i < asc_ods_str.size(); ++i) {
            ASSERT_EQ(asc_ods_str[i], expected_asc_ods_str[i]);
        }

        for (size_t i = 0; i < desc_ods_str.size(); ++i) {
            ASSERT_EQ(desc_ods_str[i], expected_desc_ods_str[i]);
        }

        for (size_t i = 0; i < simple_ods_str.size(); ++i) {
            ASSERT_EQ(simple_ods_str[i], expected_simple_ods_str[i]);
        }
    }
};

TEST_F(FastodTest, TestSmall1) {
    std::vector<std::string> expected_asc_ods_str = {
            "{} : 2<= ~ 1<=", "{} : 1<= ~ 2<=", "{} : 3<= ~ 1<=",
            "{} : 1<= ~ 3<=", "{} : 3<= ~ 2<=", "{} : 2<= ~ 3<="};

    std::vector<std::string> expected_desc_ods_str = {};

    std::vector<std::string> expected_simple_ods_str = {
            "{2} : [] -> 1<=", "{1} : [] -> 2<=", "{3} : [] -> 1<=",
            "{1} : [] -> 3<=", "{3} : [] -> 2<=", "{2} : [] -> 3<="};

    TestFastod(kOdTestNormSmall2x3, std::move(expected_asc_ods_str), std::move(expected_desc_ods_str),
               std::move(expected_simple_ods_str));
}

TEST_F(FastodTest, TestSmall2) {
    std::vector<std::string> expected_asc_ods_str = {};

    std::vector<std::string> expected_desc_ods_str = {"{} : 3>= ~ 2<=", "{} : 2>= ~ 3<="};

    std::vector<std::string> expected_simple_ods_str = {
            "{} : [] -> 1<=", "{3} : [] -> 2<=", "{2} : [] -> 3<="};

    TestFastod(kOdTestNormSmall3x3, std::move(expected_asc_ods_str), std::move(expected_desc_ods_str),
               std::move(expected_simple_ods_str));
}

TEST_F(FastodTest, TestSmall3) {
    std::vector<std::string> expected_asc_ods_str = {
            "{} : 2<= ~ 1<=", "{} : 1<= ~ 2<=", "{1} : 3<= ~ 2<=", "{1} : 2<= ~ 3<="};

    std::vector<std::string> expected_desc_ods_str = {};

    std::vector<std::string> expected_simple_ods_str = {
            "{2} : [] -> 1<=", "{3} : [] -> 1<=", "{3} : [] -> 2<=", "{2} : [] -> 3<="};

    TestFastod(kOdTestNormOd, std::move(expected_asc_ods_str), std::move(expected_desc_ods_str),
               std::move(expected_simple_ods_str));
}

TEST_F(FastodTest, TestMedium1) {
    std::vector<std::string> expected_asc_ods_str = {
            "{3} : 9<= ~ 6<=",          "{3} : 6<= ~ 9<=",          "{3} : 7<= ~ 6<=",
            "{3} : 6<= ~ 7<=",          "{3} : 1<= ~ 8<=",          "{3} : 8<= ~ 1<=",
            "{3} : 7<= ~ 4<=",          "{3} : 4<= ~ 7<=",          "{3} : 9<= ~ 4<=",
            "{3} : 4<= ~ 9<=",          "{3} : 6<= ~ 4<=",          "{3} : 4<= ~ 6<=",
            "{3} : 10<= ~ 8<=",         "{3} : 8<= ~ 10<=",         "{3} : 10<= ~ 1<=",
            "{3} : 1<= ~ 10<=",         "{3} : 5<= ~ 1<=",          "{3} : 1<= ~ 5<=",
            "{3} : 8<= ~ 5<=",          "{3} : 5<= ~ 8<=",          "{3} : 10<= ~ 5<=",
            "{3} : 5<= ~ 10<=",         "{3} : 9<= ~ 7<=",          "{3} : 7<= ~ 9<=",
            "{3,7} : 10<= ~ 6<=",       "{3,7} : 6<= ~ 10<=",       "{1,3} : 7<= ~ 5<=",
            "{1,3} : 5<= ~ 7<=",        "{1,3} : 6<= ~ 5<=",        "{1,3} : 5<= ~ 6<=",
            "{3,5} : 10<= ~ 6<=",       "{3,5} : 6<= ~ 10<=",       "{5,9} : 3<= ~ 2<=",
            "{5,9} : 2<= ~ 3<=",        "{3,9} : 6<= ~ 10<=",       "{3,9} : 10<= ~ 6<=",
            "{5,10} : 3<= ~ 2<=",       "{5,10} : 2<= ~ 3<=",       "{1,3} : 9<= ~ 5<=",
            "{1,3} : 5<= ~ 9<=",        "{1,3} : 10<= ~ 6<=",       "{1,3} : 6<= ~ 10<=",
            "{3,8} : 10<= ~ 6<=",       "{3,8} : 6<= ~ 10<=",       "{4,8} : 3<= ~ 2<=",
            "{4,8} : 2<= ~ 3<=",        "{1,3} : 10<= ~ 9<=",       "{1,3} : 9<= ~ 10<=",
            "{1,3} : 5<= ~ 4<=",        "{1,3} : 4<= ~ 5<=",        "{1,3} : 9<= ~ 8<=",
            "{1,3} : 8<= ~ 9<=",        "{1,3} : 10<= ~ 4<=",       "{1,3} : 4<= ~ 10<=",
            "{1,3} : 8<= ~ 4<=",        "{1,3} : 4<= ~ 8<=",        "{1,3} : 10<= ~ 7<=",
            "{1,3} : 7<= ~ 10<=",       "{1,3} : 8<= ~ 6<=",        "{1,3} : 6<= ~ 8<=",
            "{1,3} : 8<= ~ 7<=",        "{1,3} : 7<= ~ 8<=",        "{3,4} : 10<= ~ 6<=",
            "{3,4} : 6<= ~ 10<=",       "{4,7} : 3<= ~ 2<=",        "{4,7} : 2<= ~ 3<=",
            "{4,6} : 2<= ~ 3<=",        "{4,6} : 3<= ~ 2<=",        "{4,5,9} : 2<= ~ 6<=",
            "{4,5,9} : 6<= ~ 2<=",      "{4,5,7} : 2<= ~ 6<=",      "{4,5,7} : 6<= ~ 2<=",
            "{4,6,7} : 9<= ~ 2<=",      "{4,6,7} : 2<= ~ 9<=",      "{5,6,9} : 2<= ~ 7<=",
            "{5,7,9} : 2<= ~ 6<=",      "{5,6,9} : 7<= ~ 2<=",      "{5,7,9} : 6<= ~ 2<=",
            "{4,5,9} : 7<= ~ 2<=",      "{4,5,9} : 2<= ~ 7<=",      "{5,6,7} : 2<= ~ 3<=",
            "{5,6,7} : 3<= ~ 2<=",      "{1,5,9} : 6<= ~ 2<=",      "{1,5,9} : 2<= ~ 6<=",
            "{4,5,9} : 10<= ~ 2<=",     "{4,5,9} : 2<= ~ 10<=",     "{1,5,9} : 10<= ~ 2<=",
            "{1,5,9} : 2<= ~ 10<=",     "{1,5,6} : 3<= ~ 2<=",      "{1,5,6} : 2<= ~ 3<=",
            "{1,5,9} : 2<= ~ 4<=",      "{1,5,9} : 4<= ~ 2<=",      "{5,6,9} : 2<= ~ 10<=",
            "{5,9,10} : 2<= ~ 6<=",     "{5,9,10} : 6<= ~ 2<=",     "{5,6,9} : 10<= ~ 2<=",
            "{1,4,10} : 2<= ~ 3<=",     "{1,4,10} : 3<= ~ 2<=",     "{4,6,7} : 10<= ~ 2<=",
            "{4,6,7} : 2<= ~ 10<=",     "{5,6,7} : 10<= ~ 2<=",     "{5,6,7} : 2<= ~ 10<=",
            "{5,6,7} : 8<= ~ 2<=",      "{5,6,7} : 2<= ~ 8<=",      "{5,9,10} : 2<= ~ 7<=",
            "{5,7,9} : 2<= ~ 10<=",     "{5,9,10} : 7<= ~ 2<=",     "{5,7,9} : 10<= ~ 2<=",
            "{4,5,8} : 2<= ~ 7<=",      "{4,5,7} : 2<= ~ 8<=",      "{4,5,7} : 8<= ~ 2<=",
            "{4,5,8} : 7<= ~ 2<=",      "{1,5,9} : 7<= ~ 2<=",      "{1,5,9} : 2<= ~ 7<=",
            "{5,7,9} : 2<= ~ 8<=",      "{5,7,9} : 8<= ~ 2<=",      "{5,8,9} : 7<= ~ 2<=",
            "{5,8,9} : 2<= ~ 7<=",      "{4,9,10} : 3<= ~ 2<=",     "{4,9,10} : 2<= ~ 3<=",
            "{5,9,10} : 2<= ~ 8<=",     "{5,8,9} : 10<= ~ 2<=",     "{5,9,10} : 8<= ~ 2<=",
            "{5,8,9} : 2<= ~ 10<=",     "{5,6,8} : 2<= ~ 3<=",      "{5,6,8} : 3<= ~ 2<=",
            "{4,8,10} : 5<= ~ 2<=",     "{4,5,8} : 10<= ~ 2<=",     "{4,8,10} : 2<= ~ 5<=",
            "{4,5,8} : 2<= ~ 10<=",     "{1,5,9} : 2<= ~ 8<=",      "{1,5,9} : 8<= ~ 2<=",
            "{4,5,9} : 8<= ~ 2<=",      "{4,5,8} : 2<= ~ 9<=",      "{4,5,9} : 2<= ~ 8<=",
            "{4,5,8} : 9<= ~ 2<=",      "{4,5,8} : 6<= ~ 2<=",      "{4,5,8} : 2<= ~ 6<=",
            "{5,8,9} : 2<= ~ 6<=",      "{5,6,9} : 8<= ~ 2<=",      "{5,6,9} : 2<= ~ 8<=",
            "{5,8,9} : 6<= ~ 2<=",      "{4,5,7,8} : 6<= ~ 1<=",    "{4,5,7,8} : 1<= ~ 6<=",
            "{4,5,6,7} : 8<= ~ 1<=",    "{4,5,6,8} : 1<= ~ 7<=",    "{4,5,6,8} : 7<= ~ 1<=",
            "{4,5,6,7} : 1<= ~ 8<=",    "{4,6,7,8} : 5<= ~ 2<=",    "{4,6,7,8} : 2<= ~ 5<=",
            "{4,5,7,9} : 6<= ~ 10<=",   "{4,5,7,9} : 10<= ~ 6<=",   "{4,5,7,10} : 2<= ~ 9<=",
            "{4,5,7,10} : 9<= ~ 2<=",   "{4,5,6,10} : 7<= ~ 1<=",   "{4,5,6,10} : 1<= ~ 7<=",
            "{4,6,8,10} : 2<= ~ 9<=",   "{4,6,8,10} : 9<= ~ 2<=",   "{4,5,6,8} : 10<= ~ 3<=",
            "{4,5,6,8} : 3<= ~ 10<=",   "{4,6,9,10} : 8<= ~ 7<=",   "{4,6,9,10} : 7<= ~ 8<=",
            "{5,6,7,9} : 10<= ~ 1<=",   "{1,5,6,9} : 10<= ~ 7<=",   "{1,5,6,9} : 7<= ~ 10<=",
            "{5,6,7,9} : 1<= ~ 10<=",   "{4,5,7,9} : 10<= ~ 1<=",   "{4,5,7,9} : 1<= ~ 10<=",
            "{4,5,6,10} : 8<= ~ 7<=",   "{4,5,6,10} : 7<= ~ 8<=",   "{6,7,8,9} : 10<= ~ 2<=",
            "{6,7,8,9} : 2<= ~ 10<=",   "{4,5,6,7} : 9<= ~ 8<=",    "{4,5,6,7} : 8<= ~ 9<=",
            "{4,5,6,8} : 9<= ~ 7<=",    "{4,5,6,8} : 7<= ~ 9<=",    "{4,5,7,10} : 9<= ~ 8<=",
            "{4,5,7,10} : 8<= ~ 9<=",   "{4,5,9,10} : 8<= ~ 7<=",   "{4,5,7,9} : 10<= ~ 8<=",
            "{4,5,7,9} : 8<= ~ 10<=",   "{4,5,9,10} : 7<= ~ 8<=",   "{1,5,6,7} : 2<= ~ 4<=",
            "{1,5,6,7} : 4<= ~ 2<=",    "{1,4,5,8} : 10<= ~ 9<=",   "{1,4,5,8} : 9<= ~ 10<=",
            "{4,5,8,9} : 10<= ~ 3<=",   "{4,5,8,9} : 3<= ~ 10<=",   "{4,5,7,10} : 1<= ~ 8<=",
            "{1,4,5,10} : 7<= ~ 8<=",   "{1,4,5,10} : 8<= ~ 7<=",   "{4,5,7,10} : 8<= ~ 1<=",
            "{4,5,6,10} : 8<= ~ 1<=",   "{4,5,6,10} : 1<= ~ 8<=",   "{1,4,5,8} : 3<= ~ 10<=",
            "{1,4,5,8} : 10<= ~ 3<=",   "{4,5,6,8} : 10<= ~ 9<=",   "{4,5,6,8} : 9<= ~ 10<=",
            "{5,6,8,9} : 10<= ~ 4<=",   "{5,6,8,9} : 4<= ~ 10<=",   "{2,4,5,7} : 9<= ~ 8<=",
            "{2,4,5,7} : 8<= ~ 9<=",    "{1,5,7,10} : 4<= ~ 2<=",   "{1,5,7,10} : 2<= ~ 4<=",
            "{4,5,7,9} : 10<= ~ 3<=",   "{4,5,7,9} : 3<= ~ 10<=",   "{4,5,6,7} : 10<= ~ 3<=",
            "{4,5,6,7} : 3<= ~ 10<=",   "{4,5,7,9} : 8<= ~ 1<=",    "{4,5,7,9} : 1<= ~ 8<=",
            "{5,6,8,10} : 7<= ~ 2<=",   "{5,6,8,10} : 2<= ~ 7<=",   "{4,5,6,8} : 3<= ~ 9<=",
            "{4,5,6,8} : 9<= ~ 3<=",    "{1,5,6,8} : 4<= ~ 2<=",    "{1,5,6,8} : 2<= ~ 4<=",
            "{4,5,6,7,10} : 3<= ~ 9<=", "{4,5,6,7,10} : 9<= ~ 3<=", "{1,5,6,8,9} : 10<= ~ 3<=",
            "{1,5,6,8,9} : 3<= ~ 10<=", "{1,5,6,8,10} : 7<= ~ 9<=", "{1,5,6,8,10} : 9<= ~ 7<=",
            "{4,5,6,7,10} : 3<= ~ 8<=", "{4,5,6,7,10} : 8<= ~ 3<=", "{1,5,6,7,8} : 4<= ~ 10<=",
            "{1,4,5,7,8} : 6<= ~ 10<=", "{1,5,6,7,8} : 10<= ~ 4<=", "{1,4,5,6,8} : 10<= ~ 7<=",
            "{1,4,5,6,8} : 7<= ~ 10<=", "{1,4,5,7,8} : 10<= ~ 6<=", "{1,5,6,8,10} : 7<= ~ 4<=",
            "{1,5,6,8,10} : 4<= ~ 7<=", "{1,4,5,6,9} : 10<= ~ 3<=", "{1,4,5,6,9} : 3<= ~ 10<=",
            "{1,4,5,7,8} : 9<= ~ 3<=",  "{1,4,5,7,8} : 3<= ~ 9<=",  "{1,5,6,7,8} : 10<= ~ 3<=",
            "{1,5,6,7,8} : 3<= ~ 10<=", "{1,4,5,6,10} : 7<= ~ 2<=", "{1,4,5,6,10} : 2<= ~ 7<=",
            "{4,6,7,9,10} : 8<= ~ 5<=", "{4,6,7,9,10} : 5<= ~ 8<=", "{4,5,6,7,9} : 8<= ~ 3<=",
            "{4,5,6,7,9} : 3<= ~ 8<=",  "{1,4,5,6,10} : 9<= ~ 2<=", "{1,4,5,6,10} : 2<= ~ 9<=",
            "{4,6,8,9,10} : 1<= ~ 7<=", "{4,6,8,9,10} : 7<= ~ 1<=", "{1,4,6,8,9} : 10<= ~ 7<=",
            "{1,4,6,8,9} : 7<= ~ 10<=", "{1,4,5,6,10} : 2<= ~ 8<=", "{1,4,5,6,10} : 8<= ~ 2<=",
            "{4,6,8,9,10} : 1<= ~ 5<=", "{1,4,5,6,9} : 8<= ~ 10<=", "{1,4,5,6,9} : 10<= ~ 8<=",
            "{4,6,8,9,10} : 5<= ~ 1<=", "{1,4,6,7,9} : 5<= ~ 8<=",  "{1,4,5,6,9} : 7<= ~ 8<=",
            "{1,4,5,6,9} : 8<= ~ 7<=",  "{4,6,7,8,9} : 5<= ~ 1<=",  "{1,4,6,7,9} : 8<= ~ 5<=",
            "{1,5,6,8,9} : 4<= ~ 7<=",  "{4,6,7,8,9} : 1<= ~ 5<=",  "{1,5,6,8,9} : 7<= ~ 4<="};

    std::vector<std::string> expected_desc_ods_str = {"{3} : 7>= ~ 1<=",
                                                      "{3} : 1>= ~ 7<=",
                                                      "{3} : 4>= ~ 1<=",
                                                      "{3} : 1>= ~ 4<=",
                                                      "{3} : 9>= ~ 1<=",
                                                      "{3} : 1>= ~ 9<=",
                                                      "{3} : 6>= ~ 1<=",
                                                      "{3} : 1>= ~ 6<=",
                                                      "{4,5} : 2>= ~ 9<=",
                                                      "{4,5} : 9>= ~ 2<=",
                                                      "{5,9} : 2>= ~ 6<=",
                                                      "{5,9} : 6>= ~ 2<=",
                                                      "{4,5} : 7>= ~ 2<=",
                                                      "{4,5} : 2>= ~ 7<=",
                                                      "{5,6} : 7>= ~ 2<=",
                                                      "{5,6} : 2>= ~ 7<=",
                                                      "{5,9} : 2>= ~ 7<=",
                                                      "{5,9} : 7>= ~ 2<=",
                                                      "{3,7} : 10>= ~ 6<=",
                                                      "{3,7} : 6>= ~ 10<=",
                                                      "{3,5} : 10>= ~ 6<=",
                                                      "{3,5} : 6>= ~ 10<=",
                                                      "{3,9} : 6>= ~ 10<=",
                                                      "{3,9} : 10>= ~ 6<=",
                                                      "{3,8} : 10>= ~ 6<=",
                                                      "{3,8} : 6>= ~ 10<=",
                                                      "{5,9} : 2>= ~ 10<=",
                                                      "{5,9} : 10>= ~ 2<=",
                                                      "{4,5} : 10>= ~ 2<=",
                                                      "{4,5} : 2>= ~ 10<=",
                                                      "{5,9} : 8>= ~ 2<=",
                                                      "{5,9} : 2>= ~ 8<=",
                                                      "{5,8} : 7>= ~ 2<=",
                                                      "{5,8} : 2>= ~ 7<=",
                                                      "{5,6} : 2>= ~ 10<=",
                                                      "{5,6} : 10>= ~ 2<=",
                                                      "{3,4} : 10>= ~ 6<=",
                                                      "{3,4} : 6>= ~ 10<=",
                                                      "{5,6,9} : 4>= ~ 2<=",
                                                      "{5,6,9} : 2>= ~ 4<=",
                                                      "{4,6,9} : 7>= ~ 2<=",
                                                      "{4,6,9} : 2>= ~ 7<=",
                                                      "{5,7,9} : 2>= ~ 4<=",
                                                      "{5,7,9} : 4>= ~ 2<=",
                                                      "{5,6,7} : 2>= ~ 1<=",
                                                      "{5,6,7} : 1>= ~ 2<=",
                                                      "{4,6,9} : 10>= ~ 2<=",
                                                      "{4,6,9} : 2>= ~ 10<=",
                                                      "{5,6,9} : 1>= ~ 2<=",
                                                      "{5,6,9} : 2>= ~ 1<=",
                                                      "{1,6,9} : 10>= ~ 2<=",
                                                      "{1,6,9} : 2>= ~ 10<=",
                                                      "{5,9,10} : 4>= ~ 2<=",
                                                      "{5,9,10} : 2>= ~ 4<=",
                                                      "{5,6,10} : 2>= ~ 1<=",
                                                      "{5,6,10} : 1>= ~ 2<=",
                                                      "{5,9,10} : 1>= ~ 2<=",
                                                      "{5,9,10} : 2>= ~ 1<=",
                                                      "{4,5,7} : 1>= ~ 10<=",
                                                      "{4,5,7} : 10>= ~ 1<=",
                                                      "{4,5,10} : 6>= ~ 2<=",
                                                      "{4,5,10} : 2>= ~ 6<=",
                                                      "{4,6,9} : 1>= ~ 2<=",
                                                      "{4,6,9} : 2>= ~ 1<=",
                                                      "{1,5,9} : 2>= ~ 4<=",
                                                      "{4,5,9} : 2>= ~ 1<=",
                                                      "{4,5,9} : 1>= ~ 2<=",
                                                      "{1,5,9} : 4>= ~ 2<=",
                                                      "{4,6,9} : 8>= ~ 2<=",
                                                      "{4,6,9} : 2>= ~ 8<=",
                                                      "{4,6,8} : 10>= ~ 2<=",
                                                      "{4,6,8} : 2>= ~ 10<=",
                                                      "{5,7,9} : 1>= ~ 2<=",
                                                      "{5,7,9} : 2>= ~ 1<=",
                                                      "{4,5,7} : 2>= ~ 1<=",
                                                      "{4,5,7} : 1>= ~ 2<=",
                                                      "{4,6,7} : 2>= ~ 1<=",
                                                      "{4,6,7} : 1>= ~ 2<=",
                                                      "{4,6,8} : 1>= ~ 2<=",
                                                      "{4,6,8} : 2>= ~ 1<=",
                                                      "{4,5,10} : 1>= ~ 2<=",
                                                      "{4,5,10} : 2>= ~ 1<=",
                                                      "{4,6,10} : 1>= ~ 2<=",
                                                      "{4,6,10} : 2>= ~ 1<=",
                                                      "{4,5,6} : 2>= ~ 1<=",
                                                      "{4,5,6} : 1>= ~ 2<=",
                                                      "{4,5,10} : 8>= ~ 2<=",
                                                      "{4,5,10} : 2>= ~ 8<=",
                                                      "{4,5,8} : 1>= ~ 2<=",
                                                      "{4,5,8} : 2>= ~ 1<=",
                                                      "{5,8,9} : 1>= ~ 2<=",
                                                      "{5,8,9} : 2>= ~ 1<=",
                                                      "{5,6,8} : 1>= ~ 2<=",
                                                      "{5,6,8} : 2>= ~ 1<=",
                                                      "{5,8,9} : 4>= ~ 2<=",
                                                      "{5,8,9} : 2>= ~ 4<=",
                                                      "{4,5,8} : 6>= ~ 2<=",
                                                      "{4,5,6} : 2>= ~ 8<=",
                                                      "{4,5,6} : 8>= ~ 2<=",
                                                      "{4,5,8} : 2>= ~ 6<=",
                                                      "{4,6,7,9} : 5>= ~ 2<=",
                                                      "{4,6,7,9} : 2>= ~ 5<=",
                                                      "{4,6,7,8} : 5>= ~ 2<=",
                                                      "{4,6,7,8} : 2>= ~ 5<=",
                                                      "{4,5,7,9} : 6>= ~ 10<=",
                                                      "{4,5,7,9} : 10>= ~ 6<=",
                                                      "{1,4,6,8} : 2>= ~ 9<=",
                                                      "{1,4,6,8} : 9>= ~ 2<=",
                                                      "{4,6,7,8} : 9>= ~ 2<=",
                                                      "{4,6,7,8} : 2>= ~ 9<=",
                                                      "{4,6,7,9} : 10>= ~ 8<=",
                                                      "{4,6,7,9} : 8>= ~ 10<=",
                                                      "{4,5,9,10} : 7>= ~ 1<=",
                                                      "{4,5,9,10} : 1>= ~ 7<=",
                                                      "{4,5,6,7} : 10>= ~ 8<=",
                                                      "{4,5,7,8} : 6>= ~ 10<=",
                                                      "{4,5,6,7} : 8>= ~ 10<=",
                                                      "{4,5,6,8} : 10>= ~ 7<=",
                                                      "{4,5,6,8} : 7>= ~ 10<=",
                                                      "{4,5,7,8} : 10>= ~ 6<=",
                                                      "{4,5,6,8} : 9>= ~ 7<=",
                                                      "{4,5,6,8} : 7>= ~ 9<=",
                                                      "{4,5,8,9} : 7>= ~ 10<=",
                                                      "{4,5,8,9} : 10>= ~ 7<=",
                                                      "{4,5,7,9} : 10>= ~ 8<=",
                                                      "{4,5,7,9} : 8>= ~ 10<=",
                                                      "{1,4,6,8} : 7>= ~ 2<=",
                                                      "{1,4,6,8} : 2>= ~ 7<=",
                                                      "{1,4,5,9} : 8>= ~ 10<=",
                                                      "{4,5,8,9} : 1>= ~ 10<=",
                                                      "{1,4,5,8} : 10>= ~ 9<=",
                                                      "{1,4,5,8} : 9>= ~ 10<=",
                                                      "{4,5,8,9} : 10>= ~ 1<=",
                                                      "{1,4,5,9} : 10>= ~ 8<=",
                                                      "{1,4,5,8} : 7>= ~ 10<=",
                                                      "{1,4,5,8} : 10>= ~ 7<=",
                                                      "{4,5,6,8} : 1>= ~ 10<=",
                                                      "{4,5,6,8} : 10>= ~ 1<=",
                                                      "{4,5,7,9} : 10>= ~ 3<=",
                                                      "{4,5,7,9} : 3>= ~ 10<=",
                                                      "{4,5,6,8} : 3>= ~ 7<=",
                                                      "{4,5,6,8} : 7>= ~ 3<=",
                                                      "{4,5,6,9} : 8>= ~ 1<=",
                                                      "{4,5,6,9} : 1>= ~ 8<=",
                                                      "{1,6,8,9} : 2>= ~ 7<=",
                                                      "{1,6,8,9} : 7>= ~ 2<=",
                                                      "{4,6,7,10} : 8>= ~ 2<=",
                                                      "{4,6,7,10} : 2>= ~ 8<=",
                                                      "{4,5,6,9,10} : 3>= ~ 7<=",
                                                      "{4,5,6,9,10} : 7>= ~ 3<=",
                                                      "{1,4,6,7,10} : 9>= ~ 2<=",
                                                      "{1,4,6,7,10} : 2>= ~ 9<=",
                                                      "{1,4,6,7,9} : 10>= ~ 3<=",
                                                      "{1,4,6,7,9} : 3>= ~ 10<=",
                                                      "{5,6,7,8,9} : 10>= ~ 3<=",
                                                      "{5,6,7,8,9} : 3>= ~ 10<=",
                                                      "{1,5,6,8,9} : 10>= ~ 3<=",
                                                      "{1,5,6,8,9} : 3>= ~ 10<=",
                                                      "{1,5,6,8,9} : 10>= ~ 7<=",
                                                      "{1,5,6,8,9} : 7>= ~ 10<=",
                                                      "{1,5,7,8,10} : 6>= ~ 2<=",
                                                      "{1,5,7,8,10} : 2>= ~ 6<=",
                                                      "{1,5,6,8,9} : 7>= ~ 3<=",
                                                      "{1,5,6,8,9} : 3>= ~ 7<=",
                                                      "{1,4,5,7,8} : 3>= ~ 10<=",
                                                      "{1,4,5,7,8} : 10>= ~ 3<=",
                                                      "{1,4,5,7,8} : 6>= ~ 3<=",
                                                      "{1,4,5,7,8} : 3>= ~ 6<=",
                                                      "{4,5,6,8,10} : 7>= ~ 1<=",
                                                      "{4,5,6,7,10} : 1>= ~ 8<=",
                                                      "{4,5,6,7,10} : 8>= ~ 1<=",
                                                      "{4,5,6,8,10} : 1>= ~ 7<=",
                                                      "{1,4,5,6,9} : 10>= ~ 3<=",
                                                      "{1,4,5,6,10} : 9>= ~ 3<=",
                                                      "{1,4,5,6,9} : 3>= ~ 10<=",
                                                      "{1,4,5,6,10} : 3>= ~ 9<=",
                                                      "{1,4,5,6,8} : 10>= ~ 3<=",
                                                      "{1,4,5,6,8} : 3>= ~ 10<=",
                                                      "{1,4,6,7,10} : 5>= ~ 2<=",
                                                      "{1,4,6,7,10} : 2>= ~ 5<=",
                                                      "{1,4,5,9,10} : 7>= ~ 6<=",
                                                      "{1,4,5,6,7} : 10>= ~ 9<=",
                                                      "{1,4,6,7,9} : 10>= ~ 5<=",
                                                      "{1,4,6,7,9} : 5>= ~ 10<=",
                                                      "{1,4,5,6,9} : 7>= ~ 10<=",
                                                      "{1,4,5,9,10} : 6>= ~ 7<=",
                                                      "{1,4,5,6,7} : 9>= ~ 10<=",
                                                      "{1,4,5,6,9} : 10>= ~ 7<=",
                                                      "{4,5,6,7,10} : 8>= ~ 9<=",
                                                      "{4,5,6,7,10} : 9>= ~ 8<=",
                                                      "{1,4,5,6,9} : 7>= ~ 3<=",
                                                      "{1,5,6,7,9} : 3>= ~ 4<=",
                                                      "{1,4,5,6,7} : 9>= ~ 3<=",
                                                      "{1,4,5,6,9} : 3>= ~ 7<=",
                                                      "{1,5,6,7,9} : 4>= ~ 3<=",
                                                      "{1,4,5,6,7} : 3>= ~ 9<=",
                                                      "{4,5,7,8,9} : 3>= ~ 6<=",
                                                      "{4,5,7,8,9} : 6>= ~ 3<=",
                                                      "{5,6,7,8,9} : 3>= ~ 4<=",
                                                      "{5,6,7,8,9} : 4>= ~ 3<=",
                                                      "{1,5,6,8,9} : 10>= ~ 4<=",
                                                      "{1,5,6,8,9} : 4>= ~ 10<=",
                                                      "{1,5,6,8,9} : 3>= ~ 4<=",
                                                      "{1,5,6,8,9} : 4>= ~ 3<=",
                                                      "{1,4,5,7,8} : 9>= ~ 6<=",
                                                      "{1,4,5,7,8} : 6>= ~ 9<=",
                                                      "{4,6,7,8,9,10} : 5>= ~ 3<=",
                                                      "{4,6,7,8,9,10} : 3>= ~ 5<=",
                                                      "{2,5,6,7,8,9,10} : 1>= ~ 4<=",
                                                      "{2,5,6,7,8,9,10} : 4>= ~ 1<="};

    std::vector<std::string> expected_simple_ods_str = {
            "{3} : [] -> 2<=",           "{3,6} : [] -> 9<=",  "{3,6} : [] -> 7<=",
            "{3,9} : [] -> 5<=",         "{3,5} : [] -> 9<=",  "{3,10} : [] -> 4<=",
            "{3,7} : [] -> 1<=",         "{3,8} : [] -> 7<=",  "{3,7} : [] -> 8<=",
            "{3,8} : [] -> 1<=",         "{3,4} : [] -> 1<=",  "{3,8} : [] -> 4<=",
            "{3,4} : [] -> 8<=",         "{3,10} : [] -> 7<=", "{3,5} : [] -> 4<=",
            "{3,4} : [] -> 5<=",         "{3,10} : [] -> 9<=", "{3,7} : [] -> 4<=",
            "{3,4} : [] -> 7<=",         "{3,9} : [] -> 4<=",  "{3,4} : [] -> 9<=",
            "{3,6} : [] -> 4<=",         "{3,10} : [] -> 8<=", "{3,10} : [] -> 1<=",
            "{3,5} : [] -> 1<=",         "{3,8} : [] -> 5<=",  "{3,5} : [] -> 8<=",
            "{3,9} : [] -> 1<=",         "{3,9} : [] -> 8<=",  "{3,8} : [] -> 9<=",
            "{3,7} : [] -> 5<=",         "{3,5} : [] -> 7<=",  "{3,6} : [] -> 1<=",
            "{3,6} : [] -> 8<=",         "{3,10} : [] -> 5<=", "{3,6} : [] -> 5<=",
            "{3,9} : [] -> 7<=",         "{3,7} : [] -> 9<=",  "{1,4,5,6,8} : [] -> 9<=",
            "{1,4,5,6,7,10} : [] -> 9<="};

    TestFastod(kOdTestNormHorse10c, std::move(expected_asc_ods_str), std::move(expected_desc_ods_str),
               std::move(expected_simple_ods_str));
}

class FastodResultHashTest : public ::testing::TestWithParam<CSVConfigHash> {};

TEST_P(FastodResultHashTest, CorrectnessTest) {
    CSVConfigHash csv_config_hash = GetParam();
    size_t actual_hash = RunFastod(csv_config_hash.config);
    EXPECT_EQ(actual_hash, csv_config_hash.hash);
}

INSTANTIATE_TEST_SUITE_P(
        TestFastodSuite, FastodResultHashTest,
        ::testing::Values(
                CSVConfigHash{kOdTestNormOd, 11980520805314995804UL},
                CSVConfigHash{kOdTestNormSmall2x3, 7457915278574020764UL},
                CSVConfigHash{kOdTestNormSmall3x3, 3318291924553133612UL},
                CSVConfigHash{kOdTestNormAbalone, 8494646055080399391UL},
                CSVConfigHash{kOdTestNormBalanceScale, 0ULL},
                CSVConfigHash{kOdTestNormBreastCancerWisconsin, 16845062592796597733UL},
                CSVConfigHash{kOdTestNormClassification, 10775947267660160689UL},
                CSVConfigHash{kOdTestNormEchocardiogram, 2811588447932787109UL},
                CSVConfigHash{kOdTestNormHepatitis1, 132585063305091933UL},
                CSVConfigHash{kOdTestNormHepatitis2, 10199178000978455890UL},
                CSVConfigHash{kOdTestNormHepatitis3, 3063999440011758644UL},
                CSVConfigHash{kOdTestNormHepatitis4, 132585063305091933UL},
                CSVConfigHash{kOdTestNormHepatitis5, 11668977472753401458UL},
                CSVConfigHash{kOdTestNormHepatitis, 8347405483583260580UL},
                CSVConfigHash{kOdTestNormHorse10c, 13235589009124491858UL},
                CSVConfigHash{kOdTestNormIris, 0UL}));

}  // namespace tests
