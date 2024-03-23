#include "fastod.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

#include <boost/unordered/unordered_map.hpp>
#include <easylogging++.h>

#include "config/equal_nulls/option.h"
#include "config/names_and_descriptions.h"
#include "config/option_using.h"
#include "config/tabular_data/input_table/option.h"
#include "config/time_limit/option.h"

namespace algos {

Fastod::Fastod() : Algorithm({}) {
    PrepareOptions();
}

bool Fastod::IsTimeUp() const {
    return time_limit_seconds_ > 0 && timer_.GetElapsedSeconds() >= time_limit_seconds_;
}

void Fastod::CCPut(AttributeSet const& key, AttributeSet attribute_set) {
    cc_[key] = std::move(attribute_set);
}

fastod::AttributeSet const& Fastod::CCGet(AttributeSet const& key) {
    return cc_[key];
}

void Fastod::PrepareOptions() {
    using namespace config::names;

    RegisterOptions();
    MakeLoadOptionsAvailable();
}

void Fastod::RegisterOptions() {
    DESBORDANTE_OPTION_USING;

    RegisterOption(config::TableOpt(&input_table_));
    RegisterOption(config::TimeLimitSecondsOpt(&time_limit_seconds_));
}

void Fastod::MakeLoadOptionsAvailable() {
    using namespace config::names;
    MakeOptionsAvailable({config::TableOpt.GetName()});
}

void Fastod::MakeExecuteOptsAvailable() {
    using namespace config::names;
    MakeOptionsAvailable({config::TimeLimitSecondsOpt.GetName()});
}

void Fastod::LoadDataInternal() {
    data_ = std::make_shared<DataFrame>(DataFrame::FromInputTable(input_table_));
}

void Fastod::ResetState() {
    is_complete_ = false;

    level_ = 1;
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
    auto const start_time = std::chrono::system_clock::now();
    auto const [ods_asc, ods_desc, ods_simple] = Discover();

    auto const elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time);

    for (auto const& od : ods_asc) {
        LOG(DEBUG) << od.ToString() << '\n';
    }
    for (auto const& od : ods_desc) {
        LOG(DEBUG) << od.ToString() << '\n';
    }
    for (auto const& od : ods_simple) {
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

std::vector<fastod::AscCanonicalOD> const& Fastod::GetAscendingDependencies() const {
    return result_asc_;
}

std::vector<fastod::DescCanonicalOD> const& Fastod::GetDescendingDependencies() const {
    return result_desc_;
}

std::vector<fastod::SimpleCanonicalOD> const& Fastod::GetSimpleDependencies() const {
    return result_simple_;
}

void Fastod::Initialize() {
    timer_.Start();

    AttributeSet empty_set(data_->GetColumnCount());

    context_in_each_level_.emplace_back();
    context_in_each_level_[0].insert(empty_set);
    schema_ = AttributeSet(data_->GetColumnCount(), (1 << data_->GetColumnCount()) - 1);

    CCPut(empty_set, schema_);

    level_ = 1;
    std::unordered_set<AttributeSet> level_1_candidates;

    for (AttributeSet::SizeType i = 0; i < data_->GetColumnCount(); ++i)
        level_1_candidates.emplace(data_->GetColumnCount(), 1 << i);

    context_in_each_level_.push_back(std::move(level_1_candidates));
}

std::tuple<std::vector<fastod::AscCanonicalOD> const&, std::vector<fastod::DescCanonicalOD> const&,
           std::vector<fastod::SimpleCanonicalOD> const&>
Fastod::Discover() {
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

    return {result_asc_, result_desc_, result_simple_};
}

std::vector<std::string> Fastod::DiscoverAsStrings() {
    auto const [ods_asc, ods_desc, ods_simple] = Discover();

    std::vector<std::string> result;
    result.reserve(ods_asc.size() + ods_desc.size() + ods_simple.size());

    for (auto const& od : ods_asc) result.push_back(od.ToString());
    for (auto const& od : ods_desc) result.push_back(od.ToString());
    for (auto const& od : ods_simple) result.push_back(od.ToString());

    return result;
}

void Fastod::ComputeODs() {
    auto const& context_this_level = context_in_each_level_[level_];

    Timer timer(true);
    std::vector<std::vector<AttributeSet>> deleted_attrs(context_this_level.size());
    size_t context_ind = 0;

    for (AttributeSet const& context : context_this_level) {
        auto& del_attrs = deleted_attrs[context_ind++];
        del_attrs.reserve(data_->GetColumnCount());

        for (AttributeSet::SizeType column = 0; column < data_->GetColumnCount(); ++column) {
            del_attrs.push_back(fastod::DeleteAttribute(context, column));
        }

        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        AttributeSet context_cc = schema_;

        context.Iterate([this, &context_cc, &del_attrs](AttributeSet::SizeType attr) {
            context_cc = fastod::Intersect(context_cc, CCGet(del_attrs[attr]));
        });

        CCPut(context, context_cc);

        AddCandidates<false>(context, del_attrs);
        AddCandidates<true>(context, del_attrs);
    }

    size_t delete_index = 0;

    for (AttributeSet const& context : context_this_level) {
        auto const& delAttrs = deleted_attrs[delete_index++];

        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        AttributeSet context_intersect_cc_context = fastod::Intersect(context, CCGet(context));

        context_intersect_cc_context.Iterate(
                [this, &context, &delAttrs](AttributeSet::SizeType attr) {
                    SimpleCanonicalOD od(delAttrs[attr], attr);

                    if (od.IsValid(data_, partition_cache_)) {
                        AddToResult(std::move(od));
                        fd_count_++;

                        CCPut(context, fastod::DeleteAttribute(CCGet(context), attr));

                        const AttributeSet diff = fastod::Difference(schema_, context);

                        diff.Iterate([this, &context](AttributeSet::SizeType i) {
                            CCPut(context, fastod::DeleteAttribute(CCGet(context), i));
                        });
                    }
                });

        CalculateODs<false>(context, delAttrs);
        CalculateODs<true>(context, delAttrs);
    }
}

void Fastod::PruneLevels() {
    if (level_ >= 2) {
        auto& contexts = context_in_each_level_[level_];

        for (auto attribute_set_it = contexts.begin(); attribute_set_it != contexts.end();) {
            if (IsEmptySet(CCGet(*attribute_set_it)) && CSGet<true>(*attribute_set_it).empty() &&
                CSGet<false>(*attribute_set_it).empty()) {
                contexts.erase(attribute_set_it++);
            } else {
                ++attribute_set_it;
            }
        }
    }
}

void Fastod::CalculateNextLevel() {
    boost::unordered_map<AttributeSet, std::vector<size_t>> prefix_blocks;
    std::unordered_set<AttributeSet> context_next_level;

    auto const& context_this_level = context_in_each_level_[level_];

    for (AttributeSet const& attribute_set : context_this_level) {
        attribute_set.Iterate([&prefix_blocks, &attribute_set](AttributeSet::SizeType attr) {
            prefix_blocks[fastod::DeleteAttribute(attribute_set, attr)].push_back(attr);
        });
    }

    for (auto const& [prefix, single_attributes] : prefix_blocks) {
        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        if (single_attributes.size() <= 1) {
            continue;
        }

        for (size_t i = 0; i < single_attributes.size(); ++i) {
            for (size_t j = i + 1; j < single_attributes.size(); ++j) {
                bool create_context = true;

                const AttributeSet candidate = fastod::AddAttribute(
                        fastod::AddAttribute(prefix, single_attributes[i]), single_attributes[j]);

                candidate.Iterate([&context_this_level, &candidate,
                                   &create_context](AttributeSet::SizeType attr) {
                    if (context_this_level.find(fastod::DeleteAttribute(candidate, attr)) ==
                        context_this_level.end()) {
                        create_context = false;
                        return;
                    }
                });

                if (create_context) {
                    context_next_level.insert(candidate);
                }
            }
        }
    }

    context_in_each_level_.push_back(std::move(context_next_level));
}

}  // namespace algos
