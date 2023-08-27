#include <iostream>
#include <utility>
#include <set>
#include <map>
#include <algorithm>

#include "fastod.h"
#include "operator_type.h"
#include "single_attribute_predicate.h"
#include "stripped_partition.h"

using namespace algos::fastod;

Fastod::Fastod(const DataFrame& data, long time_limit, double error_rate_threshold) noexcept :
    //Algorithm({"Mining ODs"}),
    data_(std::move(data)),
    time_limit_(time_limit),
    error_rate_threshold_(error_rate_threshold) {}

Fastod::Fastod(const DataFrame& data, long time_limit) noexcept :
    //Algorithm({"Mining ODs"}),
    data_(std::move(data)),
    time_limit_(time_limit) {}

bool Fastod::IsTimeUp() const noexcept {
    return timer_.GetElapsedSeconds() >= time_limit_;
}

void Fastod::CCPut(size_t key, size_t attribute_set) noexcept {
    cc_[key] = attribute_set;
}

size_t Fastod::CCGet(size_t key) noexcept {
    const auto it = cc_.find(key);
    if (it != cc_.cend())
        return it->second;
    return cc_[key] = 0;
}

void Fastod::CSPut(size_t key, const AttributePair& value) noexcept {
    const auto it = cs_.find(key);
    if (it != cs_.cend())
        it->second.emplace(value);
    else
        cs_[key] = {value};
}

void Fastod::CSPut(size_t key, AttributePair&& value) noexcept {
    const auto it = cs_.find(key);
    if (it != cs_.cend())
        it->second.emplace(std::move(value));
    else
        cs_[key] = {std::move(value)};
}

std::unordered_set<AttributePair>& Fastod::CSGet(size_t key) noexcept {
    const auto it = cs_.find(key);
    if (it != cs_.cend())
        return it->second;
    return cs_[key] = {};
}

void Fastod::PrintStatistics() const noexcept {
    std::string last_od = result_.size() > 0
        ? result_[result_.size() - 1].ToString()
        : std::string("");
    
    std::cout << "Current time " << timer_.GetElapsedSeconds() << " sec, "
              << "found od " << fd_count_ + ocd_count_ << " ones, where "
              << "fd " << fd_count_ << " ones, "
              << "ocd " << ocd_count_ << " ones, "
              << "the last od is " << last_od << '\n';
}

bool Fastod::IsComplete() const noexcept {
    return is_complete_;
}

void Fastod::Initialize() noexcept {
    timer_.Start();

    size_t empty_set = 0;

    context_in_each_level_.push_back({});
    context_in_each_level_[0].insert(empty_set);
    size_t put_value = 0;
    for (size_t i = 0; i < data_.GetColumnCount(); i++) {
        schema_ = addAttribute(schema_, i);
        put_value += 1 << i;
    }
    CCPut(empty_set, put_value);

    level_ = 1;
    std::unordered_set<size_t> level_1_candidates;

    for (size_t i = 0; i < data_.GetColumnCount(); i++)
        level_1_candidates.emplace(1 << i);

    context_in_each_level_.emplace_back(std::move(level_1_candidates));
}

std::vector<CanonicalOD> Fastod::Discover() noexcept {
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
        std::cout << "FastOD finished successfully" << '\n';
    } else {
        std::cout << "FastOD finished with a time-out" << '\n';
    }

    std::cout << "Seconds elapsed: " << timer_.GetElapsedSeconds() << '\n'
              << "ODs found: " << fd_count_ + ocd_count_ << '\n'
              << "FDs found: " << fd_count_ << '\n'
              << "OCDs found: " << ocd_count_ << '\n';
    return result_;
}

void Fastod::ComputeODs() noexcept {
    const auto& context_this_level = context_in_each_level_[level_];
    Timer timer(true);
    for (size_t context : context_this_level) {
        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        size_t context_cc = schema_;
        for (ASIterator it = attrsBegin(context); it != attrsEnd(context); ++it) {
            context_cc = intersect(context_cc, CCGet(deleteAttribute(context, *it)));
        }

        CCPut(context, context_cc);

        if (level_ == 2) {
            for (size_t i = 0; i < data_.GetColumnCount(); i++) {
                for (size_t j = 0; j < data_.GetColumnCount(); j++) {
                    if (i == j) {
                        continue;
                    }
                    size_t c = attributeSet({i, j});

                    CSPut(c, AttributePair(SingleAttributePredicate::GetInstance(i, Operator(OperatorType::GreaterOrEqual)), j));
                    CSPut(c, AttributePair(SingleAttributePredicate::GetInstance(i, Operator(OperatorType::LessOrEqual)), j));
                }
            }
        } else if (level_ > 2) {
            std::unordered_set<AttributePair> candidate_cs_pair_set;
            for (ASIterator attr = attrsBegin(context); attr != attrsEnd(context); ++attr) {
                for (const AttributePair& pair : CSGet(deleteAttribute(context, *attr))) {
                    candidate_cs_pair_set.emplace(pair);
                }
            }

            for (AttributePair const& attribute_pair : candidate_cs_pair_set) {
                size_t context_delete_ab = deleteAttribute(deleteAttribute(context,
                    attribute_pair.GetLeft().GetAttribute()), attribute_pair.GetRight());

                bool add_context = true;
                for (ASIterator attr = attrsBegin(context_delete_ab);
                    attr != attrsEnd(context_delete_ab); ++attr) {
                    if (CSGet(deleteAttribute(context, *attr)).count(attribute_pair) == 0) {
                        add_context = false;
                        break;
                    }
                }

                if (add_context) {
                    CSPut(context, attribute_pair);
                }
            }
        }
    }

    for (size_t context : context_this_level) {
        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        size_t context_intersect_cc_context = intersect(context, CCGet(context));
        for (ASIterator attr = attrsBegin(context_intersect_cc_context);
            attr != attrsEnd(context_intersect_cc_context); ++attr) {
            CanonicalOD od(deleteAttribute(context, *attr), *attr);

            if (od.IsValid(data_, error_rate_threshold_)) {
                result_.emplace_back(std::move(od));
                fd_count_++;
                CCPut(context, deleteAttribute(CCGet(context), *attr));

                size_t diff = difference(schema_, context);
                for (ASIterator i = attrsBegin(diff); i != attrsEnd(diff); ++i)
                    CCPut(context, deleteAttribute(CCGet(context), *i));

                //PrintStatistics();
            }
        }
        

        auto& cs_for_con = CSGet(context);
        for (auto it = cs_for_con.begin(); it != cs_for_con.end();) {
            size_t a = it->GetLeft().GetAttribute();
            size_t b = it->GetRight();

            if (containsAttribute(CCGet(deleteAttribute(context, b)), a) &&
                containsAttribute(CCGet(deleteAttribute(context, a)), b)) {
                CanonicalOD od(deleteAttribute(deleteAttribute(context, a), b), it->GetLeft(), b);
                
                if (od.IsValid(data_, error_rate_threshold_)) {
                    ++ocd_count_;
                    result_.emplace_back(std::move(od));
                    cs_for_con.erase(it++);
                } else {
                    ++it;
                }
            } else {
                cs_for_con.erase(it++);
            }
        }
    }
}

void Fastod::PruneLevels() noexcept {
    if (level_ >= 2) {
        auto& contexts = context_in_each_level_[level_];

        for (auto attribute_set_it = contexts.begin();
            attribute_set_it != contexts.end();) {
            if (isEmptyAS(CCGet(*attribute_set_it)) && CSGet(*attribute_set_it).empty())
                contexts.erase(attribute_set_it++);
            else
                ++attribute_set_it;
        }
    }
}

void Fastod::CalculateNextLevel() noexcept {
    std::unordered_map<size_t, std::vector<size_t>> prefix_blocks;
    std::unordered_set<size_t> context_next_level;

    const auto& context_this_level = context_in_each_level_[level_];

    for (size_t attribute_set : context_this_level) {
        for (ASIterator attr = attrsBegin(attribute_set); 
            attr != attrsEnd(attribute_set); ++attr) {
            size_t prefix = deleteAttribute(attribute_set, *attr);

            if (prefix_blocks.count(prefix) == 0)
                prefix_blocks[prefix] = std::vector<size_t>();

            prefix_blocks[prefix].push_back(*attr);
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
                size_t candidate = addAttribute(addAttribute(prefix,
                    single_attributes[i]), single_attributes[j]);
                for (ASIterator attr = attrsBegin(candidate); 
                    attr != attrsEnd(candidate); ++attr) {
                    if (context_this_level.count(deleteAttribute(candidate, *attr)) == 0) {
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

    context_in_each_level_.emplace_back(std::move(context_next_level));
}