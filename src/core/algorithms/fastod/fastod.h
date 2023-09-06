#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

#include "attribute_set.h"
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
    size_t od_count_ = 0;
    size_t fd_count_ = 0;
    size_t ocd_count_ = 0;

    std::vector<CanonicalOD> result_;
    std::vector<std::unordered_set<AttributeSet>> context_in_each_level_;
    std::unordered_map<AttributeSet, AttributeSet> cc_;
    std::unordered_map<AttributeSet, std::unordered_set<AttributePair>> cs_;
    StrippedPartitionCache partition_cache_;

    AttributeSet schema_;
    DataFrame data_;

    Timer timer_;

    void addToRes(CanonicalOD&& od);
    bool IsTimeUp() const;
    
    void CCPut(AttributeSet const& key, AttributeSet attribute_set);
    AttributeSet const& CCGet(AttributeSet const& key);
    void CSPut(AttributeSet const& key, const AttributePair& value);
    void CSPut(AttributeSet const& key, AttributePair&& value);
    std::unordered_set<AttributePair>& CSGet(AttributeSet const& key);

    void Initialize();

    void ComputeODs();
    void PruneLevels();
    void CalculateNextLevel();

public:
    Fastod(DataFrame data, long time_limit);

    void PrintStatistics() const;
    bool IsComplete() const;
    std::vector<CanonicalOD> Discover();
};

} // namespace algos::fatod
