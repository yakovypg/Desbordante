#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

#include "canonical_od.h"
#include "attribute_pair.h"
#include "timer.h"
#include "stripped_partition_cache.h"

namespace algos::fastod {

class Fastod {
private:
    const long time_limit_;
    bool is_complete_ = true;
    size_t level_;
    double error_rate_threshold_ = -1;
    size_t od_count_ = 0;
    size_t fd_count_ = 0;
    size_t ocd_count_ = 0;

    std::vector<CanonicalOD> result_;
    std::vector<std::unordered_set<size_t>> context_in_each_level_;
    std::unordered_map<size_t, size_t> cc_;
    std::unordered_map<size_t, std::unordered_set<AttributePair>> cs_;
    StrippedPartitionCache partition_cache_;

    size_t schema_;
    const DataFrame& data_;

    Timer timer_;

    void addToRes(CanonicalOD&& od);
    bool IsTimeUp() const;
    
    void CCPut(size_t key, size_t attribute_set);
    size_t CCGet(size_t key);
    void CSPut(size_t key, const AttributePair& value);
    void CSPut(size_t key, AttributePair&& value);
    std::unordered_set<AttributePair>& CSGet(size_t key);

    void Initialize();

    void ComputeODs();
    void PruneLevels();
    void CalculateNextLevel();

public:
    Fastod(const DataFrame& data, long time_limit, double error_rate_threshold) :
        time_limit_(time_limit), error_rate_threshold_(error_rate_threshold), data_(std::move(data)) {}
    Fastod(const DataFrame& data, long time_limit, size_t threads = 1) :
        time_limit_(time_limit), data_(std::move(data)) {}

    void PrintStatistics() const;
    bool IsComplete() const;
    std::vector<CanonicalOD> Discover();
};

} // namespace algos::fatod
