#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

#include "canonical_od.h"
#include "attribute_pair.h"
#include "timer.h"
#include "stripped_partition_cache.h"

#include <mutex>
#include <shared_mutex>

namespace algos::fastod {

template <bool multithread>
class Fastod/*: public Algorithm*/ {
private:
    const long time_limit_;
    bool is_complete_ = true;
    size_t level_;
    double error_rate_threshold_ = -1;
    size_t od_count_ = 0;
    size_t fd_count_ = 0;
    size_t ocd_count_ = 0;
    size_t threads_num_ = 1;

    std::mutex m_result_;
    std::shared_mutex m_cc_, m_cs_;

    std::vector<CanonicalOD<multithread>> result_;
    std::vector<std::unordered_set<size_t>> context_in_each_level_;
    std::unordered_map<size_t, size_t> cc_;
    std::unordered_map<size_t, std::unordered_set<AttributePair>> cs_;
    StrippedPartitionCache<multithread> partition_cache_;

    size_t schema_;
    const DataFrame& data_;

    Timer timer_;

    // void PrintState() const noexcept;

    void addToRes(CanonicalOD<multithread>&& od);
    bool IsTimeUp() const noexcept;
    
    void CCPut(size_t key, size_t attribute_set) noexcept;
    size_t CCGet(size_t key) noexcept;
    void CSPut(size_t key, const AttributePair& value) noexcept;
    void CSPut(size_t key, AttributePair&& value) noexcept;
    std::unordered_set<AttributePair>& CSGet(size_t key) noexcept;

    void Initialize() noexcept;

    void ComputeODs() noexcept;
    void PruneLevels() noexcept;
    void CalculateNextLevel() noexcept;

public:
    Fastod(const DataFrame& data, long time_limit, double error_rate_threshold, size_t threads = 1) noexcept :
        time_limit_(time_limit), error_rate_threshold_(error_rate_threshold),
        threads_num_(threads), data_(std::move(data)) {}
    Fastod(const DataFrame& data, long time_limit, size_t threads = 1) noexcept :
        time_limit_(time_limit), threads_num_(threads), data_(std::move(data)) {}

    void PrintStatistics() const noexcept;
    bool IsComplete() const noexcept;
    std::vector<CanonicalOD<multithread>> Discover() noexcept;
};

using SimpleFastOD = Fastod<false>;
using MutiFastOD = Fastod<true>;

} // namespace algos::fatod
