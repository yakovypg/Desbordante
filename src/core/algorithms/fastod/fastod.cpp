#include <iostream>
#include <utility>
#include <algorithm>

#include "fastod.h"
#include "single_attribute_predicate.h"
#include "stripped_partition.h"

namespace algos::fastod {

Fastod::Fastod(DataFrame data, long time_limit) :
    time_limit_(time_limit), data_(std::move(data)) {}

bool Fastod::IsTimeUp() const {
    return timer_.GetElapsedSeconds() >= time_limit_;
}

void Fastod::CCPut(AttributeSet const& key, AttributeSet attribute_set) {
    cc_[key] = std::move(attribute_set);
}

AttributeSet const& Fastod::CCGet(AttributeSet const& key) {
    const auto it = cc_.find(key);
    if (it != cc_.cend())
        return it->second;
    return cc_[key] = AttributeSet(data_.GetColumnCount());
}

void Fastod::addToRes(CanonicalOD&& od) {
    result_.emplace_back(std::move(od));
}

void Fastod::CSPut(AttributeSet const& key, const AttributePair& value) {
    cs_[key].emplace(value);
}

void Fastod::CSPut(AttributeSet const& key, AttributePair&& value) {
    cs_[key].emplace(std::move(value));
}

std::unordered_set<AttributePair>& Fastod::CSGet(AttributeSet const& key) {
    return cs_[key];
}

void Fastod::PrintStatistics() const {
    std::string last_od = result_.size() > 0
        ? result_[result_.size() - 1].ToString()
        : std::string("");
    std::cout << "RESULT: Time=" << timer_.GetElapsedSeconds() << ", "
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

    for (size_t i = 0; i < data_.GetColumnCount(); i++)
        level_1_candidates.emplace(data_.GetColumnCount(), 1 << i);

    context_in_each_level_.push_back(std::move(level_1_candidates));
}

std::vector<CanonicalOD> Fastod::Discover() {
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
    PrintStatistics();
    return result_;
}

void Fastod::ComputeODs() {
    const auto& context_this_level = context_in_each_level_[level_];
    Timer timer(true);
    std::vector<std::vector<AttributeSet>> deletedAttrs(context_this_level.size());
    size_t contextInd = 0;
    for (AttributeSet const& context : context_this_level) {
        auto& delAttrs = deletedAttrs[contextInd++];
        delAttrs.reserve(data_.GetColumnCount());
        for (size_t col = 0; col < data_.GetColumnCount(); ++col)
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

        if (level_ == 2) {
            for (size_t i = 0; i < data_.GetColumnCount(); i++) {
                for (size_t j = 0; j < data_.GetColumnCount(); j++) {
                    if (i == j) {
                        continue;
                    }
                    AttributeSet c = attributeSet({i, j}, data_.GetColumnCount());

                    CSPut(c, AttributePair(SingleAttributePredicate(i, false), j));
                    CSPut(c, AttributePair(SingleAttributePredicate(i, true), j));
                }
            }
        } else if (level_ > 2) {
            std::unordered_set<AttributePair> candidate_cs_pair_set;
            for (AttributeSet::size_type attr = context.find_first(); attr != AttributeSet::npos;
                 attr = context.find_next(attr)) {
                const auto& tmp = CSGet(delAttrs[attr]);
                candidate_cs_pair_set.insert(tmp.cbegin(), tmp.cend());
            }

            for (AttributePair const& attribute_pair : candidate_cs_pair_set) {
                AttributeSet context_delete_ab = deleteAttribute(
                    delAttrs[attribute_pair.left.attribute], attribute_pair.right);

                bool add_context = true;
                for (AttributeSet::size_type attr = context_delete_ab.find_first();
                    attr != AttributeSet::npos; attr = context_delete_ab.find_next(attr)) {
                    if (CSGet(delAttrs[attr]).count(attribute_pair) == 0) {
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
            CanonicalOD od(delAttrs[attr], attr);

            if (od.IsValid(data_, partition_cache_)) {
                addToRes(std::move(od));
                fd_count_++;
                CCPut(context, deleteAttribute(CCGet(context), attr));

                AttributeSet diff = difference(schema_, context);
                for (AttributeSet::size_type i = diff.find_first(); i != AttributeSet::npos;
                     i = diff.find_next(i))
                    CCPut(context, deleteAttribute(CCGet(context), i));
                //PrintStatistics();
            }
        }

        auto& cs_for_con = CSGet(context);
        for (auto it = cs_for_con.begin(); it != cs_for_con.end();) {
            size_t a = it->left.attribute;
            size_t b = it->right;

            if (containsAttribute(CCGet(delAttrs[b]), a) &&
                containsAttribute(CCGet(delAttrs[a]), b)) {
                CanonicalOD od(deleteAttribute(delAttrs[a], b), it->left, b);
                if (od.IsValid(data_, partition_cache_)) {
                    ++ocd_count_;
                    addToRes(std::move(od));
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

void Fastod::PruneLevels() {
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

void Fastod::CalculateNextLevel() {
    std::unordered_map<AttributeSet, std::vector<size_t>> prefix_blocks;
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

}
