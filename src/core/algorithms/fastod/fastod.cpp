#include <iostream>
#include <utility>
#include <algorithm>

#include <easylogging++.h>
#include <boost/unordered/unordered_map.hpp>

#include "fastod.h"
#include "single_attribute_predicate.h"
#include "stripped_partition.h"

namespace algos::fastod {

Fastod::Fastod(DataFrame data, long time_limit)
    : time_limit_(time_limit), data_(std::move(data)), Algorithm({}) { }

bool Fastod::IsTimeUp() const {
    return timer_.GetElapsedSeconds() >= time_limit_;
}

void Fastod::CCPut(AttributeSet const& key, AttributeSet attribute_set) {
    cc_[key] = std::move(attribute_set);
}

AttributeSet const& Fastod::CCGet(AttributeSet const& key) {
    return cc_[key];
}

void Fastod::LoadDataInternal() {
}

void Fastod::ResetState() {
    is_complete_ = false;

    level_ = 0;
    od_count_ = 0;
    fd_count_ = 0;
    ocd_count_ = 0;

    result_asc_.clear();
    result_desc_.clear();
    result_simple_.clear();
    context_in_each_level_.clear();
    cc_.clear();
    cs_asc_.clear();
    cs_desc_.clear();

    timer_ = Timer();
    partition_cache_.Clear();
}

unsigned long long Fastod::ExecuteInternal() {
    auto start_time = std::chrono::system_clock::now();
    auto [odsAsc, odsDesc, odsSimple] = Discover();

    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - start_time);

    for (const auto& od : odsAsc) {
        LOG(DEBUG) << od.ToString() << '\n';
    }
    for (const auto& od : odsDesc) {
        LOG(DEBUG) << od.ToString() << '\n';
    }
    for (const auto& od : odsSimple) {
        LOG(DEBUG) << od.ToString() << '\n';
    }

    return elapsed_milliseconds.count();
}

void Fastod::PrintStatistics() const {
    LOG(DEBUG) << "RESULT: Time=" << timer_.GetElapsedSeconds() << ", "
               << "OD=" << fd_count_ + ocd_count_ << ", "
               << "FD=" << fd_count_ << ", "
               << "OCD=" << ocd_count_ << '\n';
}

bool Fastod::IsComplete() const {
    return is_complete_;
}

void Fastod::Initialize() {
    timer_.Start();

    AttributeSet empty_set(data_.GetColumnCount());

    context_in_each_level_.push_back({});
    context_in_each_level_[0].insert(empty_set);
    schema_ = AttributeSet(data_.GetColumnCount(), (1 << data_.GetColumnCount()) - 1);
    CCPut(empty_set, schema_);

    level_ = 1;
    std::unordered_set<AttributeSet> level_1_candidates;

    for (AttributeSet::size_type i = 0; i < data_.GetColumnCount(); ++i)
        level_1_candidates.emplace(data_.GetColumnCount(), 1 << i);

    context_in_each_level_.push_back(std::move(level_1_candidates));
}

std::tuple<std::vector<CanonicalOD<true>>,
           std::vector<CanonicalOD<false>>,
           std::vector<SimpleCanonicalOD>> Fastod::Discover() {
    Initialize();

    while (!context_in_each_level_[level_].empty()) {
        ComputeODs();
        if (IsTimeUp()) {
            break;
        }
        PruneLevels();
        CalculateNextLevel();
        if (IsTimeUp()) {
            break;
        }

        level_++;
    }

    timer_.Stop();

    if (IsComplete()) {
        LOG(DEBUG) << "FastOD finished successfully" << '\n';
    } else {
        LOG(DEBUG) << "FastOD finished with a time-out" << '\n';
    }
    PrintStatistics();
    return { std::move(result_asc_), std::move(result_desc_), std::move(result_simple_) };
}

std::vector<std::string> Fastod::DiscoverAsStrings() {
    auto [odsAsc, odsDesc, odsSimple] = Discover();
    std::vector<std::string> result;
    result.reserve(odsAsc.size() + odsDesc.size() + odsSimple.size());
    for (const auto& od : odsAsc)
        result.push_back(od.ToString());
    for (const auto& od : odsDesc)
        result.push_back(od.ToString());
    for (const auto& od : odsSimple)
        result.push_back(od.ToString());
    return result;
}

void Fastod::ComputeODs() {
    const auto& context_this_level = context_in_each_level_[level_];
    Timer timer(true);
    std::vector<std::vector<AttributeSet>> deletedAttrs(context_this_level.size());
    size_t contextInd = 0;
    for (AttributeSet const& context : context_this_level) {
        auto& delAttrs = deletedAttrs[contextInd++];
        delAttrs.reserve(data_.GetColumnCount());
        for (AttributeSet::size_type col = 0; col < data_.GetColumnCount(); ++col)
            delAttrs.push_back(deleteAttribute(context, col));
        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        AttributeSet context_cc = schema_;
        for (AttributeSet::size_type attr = context.find_first(); attr != AttributeSet::npos;
             attr = context.find_next(attr)) {
            context_cc = intersect(context_cc, CCGet(delAttrs[attr]));
        }

        CCPut(context, context_cc);

        AddCandidates<false>(context, delAttrs);
        AddCandidates<true>(context, delAttrs);
    }
    size_t condInd = 0;
    for (AttributeSet const& context : context_this_level) {
        const auto& delAttrs = deletedAttrs[condInd++];
        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }
        
        AttributeSet context_intersect_cc_context = intersect(context, CCGet(context));
        for (AttributeSet::size_type attr = context_intersect_cc_context.find_first();
             attr != AttributeSet::npos; attr = context_intersect_cc_context.find_next(attr)) {
            SimpleCanonicalOD od(delAttrs[attr], attr);

            if (od.IsValid(data_, partition_cache_)) {
                addToRes(std::move(od));
                fd_count_++;
                CCPut(context, deleteAttribute(CCGet(context), attr));

                AttributeSet diff = difference(schema_, context);
                for (AttributeSet::size_type i = diff.find_first(); i != AttributeSet::npos;
                     i = diff.find_next(i))
                    CCPut(context, deleteAttribute(CCGet(context), i));
            }
        }
        CalcODs<false>(context, delAttrs);
        CalcODs<true>(context, delAttrs);
        
    }
}

void Fastod::PruneLevels() {
    if (level_ >= 2) {
        auto& contexts = context_in_each_level_[level_];

        for (auto attribute_set_it = contexts.begin();
            attribute_set_it != contexts.end();) {
            if (isEmptyAS(CCGet(*attribute_set_it)) &&
                 CSGet<true>(*attribute_set_it).empty() &&
                 CSGet<false>(*attribute_set_it).empty())
                contexts.erase(attribute_set_it++);
            else
                ++attribute_set_it;
        }
    }
}

void Fastod::CalculateNextLevel() {
    boost::unordered_map<AttributeSet, std::vector<size_t>> prefix_blocks;
    std::unordered_set<AttributeSet> context_next_level;

    const auto& context_this_level = context_in_each_level_[level_];

    for (AttributeSet const& attribute_set : context_this_level) {
        for (AttributeSet::size_type attr = attribute_set.find_first();
            attr != AttributeSet::npos; attr = attribute_set.find_next(attr)) {
            prefix_blocks[deleteAttribute(attribute_set, attr)].push_back(attr);
        }
    }

    for (auto const& [prefix, single_attributes] : prefix_blocks) {
        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        if (single_attributes.size() <= 1)
            continue;

        for (size_t i = 0; i < single_attributes.size(); ++i) {
            for (size_t j = i + 1; j < single_attributes.size(); ++j) {
                bool create_context = true;
                AttributeSet candidate = addAttribute(addAttribute(prefix,
                    single_attributes[i]), single_attributes[j]);
                for (AttributeSet::size_type attr = candidate.find_first();
                    attr != AttributeSet::npos; attr = candidate.find_next(attr)) {
                    if (context_this_level.count(deleteAttribute(candidate, attr)) == 0) {
                        create_context = false;
                        break;
                    }
                }

                if (create_context) {
                    context_next_level.insert(candidate);
                }
            }
        }
    }

    context_in_each_level_.push_back(std::move(context_next_level));
}

} // namespace algos::fastod
