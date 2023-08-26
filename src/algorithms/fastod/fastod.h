#pragma once

#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>

#include "algorithm.h"
#include "attribute_pair.h"
#include "attribute_set.h"
#include "canonical_od.h"
#include "data_frame.h"
#include "timer.h"

namespace algos::fastod {

class Fastod/*: public Algorithm*/ {
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

    size_t schema_;
    const DataFrame& data_;

    Timer timer_;

    // void PrintState() const noexcept;

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
    Fastod(const DataFrame& data, long time_limit, double error_rate_threshold) noexcept;
    Fastod(const DataFrame& data, long time_limit) noexcept;

    void PrintStatistics() const noexcept;
    bool IsComplete() const noexcept;
    std::vector<CanonicalOD> Discover() noexcept;
};

} // namespace algos::fatod
