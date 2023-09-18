#include <iostream>
#include <utility>
#include <algorithm>
#include <thread>
#include <chrono>

#include <boost/unordered/unordered_map.hpp>

#include "fastod.h"
#include "single_attribute_predicate.h"
#include "stripped_partition.h"

using namespace std::chrono_literals;

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
    return cc_[key];
}

void Fastod::PrintStatistics() const {
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

    for (AttributeSet::size_type i = 0; i < data_.GetColumnCount(); ++i)
        level_1_candidates.emplace(data_.GetColumnCount(), 1 << i);

    context_in_each_level_.push_back(std::move(level_1_candidates));
}

std::size_t max_available_level = 1;
std::size_t max_level = 0;
bool is_max_level_calculated = false;
bool is_curr_level_computed = false;

// void Fastod::CalculateAllLevels() {
//     std::size_t current_level = 1;

//     while (!context_in_each_level_[current_level].empty()) {
//         //PruneLevels();
//         CalculateNextLevel(current_level);

//         ++current_level;
//         ++max_available_level;
//     }

//     max_level = current_level - 1;
//     max_available_level = max_level;
//     is_max_level_calculated = true;
// }

// void Fastod::ComputeODs(std::size_t level_from, std::size_t level_to) {   
//     while (level_from <= level_to) {
//         ComputeODs(level_from++);
//     }
// }

void Fastod::CalculateAllLevels() {
    std::size_t current_level = 1;

    while (!context_in_each_level_[current_level].empty()) {
        //std::cout << is_curr_level_computed << "\n";
        std::this_thread::sleep_for(40ms);
        if (is_curr_level_computed) {
            //std::cout << "Prune " << current_level << "\n";
            PruneLevels(current_level);
            //std::cout << "Calc " << current_level << "\n";
            CalculateNextLevel(current_level);

            ++current_level;
            ++max_available_level;

            is_curr_level_computed = false;
        }
    }

    max_level = current_level - 1;
    max_available_level = max_level;
    is_max_level_calculated = true;
}

void Fastod::ComputeODs(std::size_t level_from, std::size_t level_to) {   
    while (level_from <= level_to) {
        ComputeODs(level_from++);
    }
}

void Fastod::ComputeODsThread() {
    std::size_t current_level = 1;
    
    while (!is_max_level_calculated || current_level <= max_level) {       
        //std::cout << current_level << " < " << max_available_level << " == " << (current_level <= max_available_level) << "\n";
        std::this_thread::sleep_for(30ms);
        if (current_level <= max_available_level) {
            //std::cout << "Compute " << current_level << "\n";
            ComputeODs(current_level++);
            is_curr_level_computed = true;
        }
    }
}

std::tuple<std::vector<CanonicalOD<true>>,
           std::vector<CanonicalOD<false>>,
           std::vector<SimpleCanonicalOD>> Fastod::Discover() {
    Initialize();

    // std::thread t1(&Fastod::Compute1, this);
    // std::thread t2(&Fastod::Compute2, this);
    // t1.join();
    // t2.join();

    // Timer timer1(true);
    // Compute1();
    // std::cout << timer1.GetElapsedSeconds() << std::endl;
    // Timer timer2(true);
    // Compute2();
    // std::cout << timer2.GetElapsedSeconds() << std::endl;

    //Compute1();
    //Compute2();

    //CalculateAllLevels();
    // ComputeODs(1, 5);
    // ComputeODs(9, 9);
    // ComputeODs(6, 7);
    // ComputeODs(12, 15);
    // ComputeODs(8, 8);
    // ComputeODs(16, 18);
    // ComputeODs(10, 11);
    // ComputeODs(19, 21);

    // ComputeODs(1, 5);
    // ComputeODs(6, 7);
    // ComputeODs(8, 8);
    // ComputeODs(9, 9);
    // ComputeODs(10, 11);
    // ComputeODs(12, 15);
    // ComputeODs(16, 18);
    // ComputeODs(19, 21);

    // ComputeODs(19, 21);
    // ComputeODs(16, 18);
    // ComputeODs(12, 15);
    // ComputeODs(10, 11);
    // ComputeODs(8, 9);
    // ComputeODs(6, 7);
    // ComputeODs(1, 5);

    // std::thread t1(&Fastod::CalculateAllLevels, this);
    // std::this_thread::sleep_for(40ms);
    // std::thread t2(&Fastod::ComputeODsThread, this);
    // t1.join();
    // t2.join();

    std::thread t2(&Fastod::ComputeODsThread, this);
    std::this_thread::sleep_for(40ms);
    std::thread t1(&Fastod::CalculateAllLevels, this);
    t2.join();
    t1.join();

    // std::thread t1(&Fastod::CalculateAllLevels, this);
    // t1.join();
    // std::thread t2(&Fastod::ComputeODsThread, this);
    // t2.join();

    // CalculateAllLevels();
    // ComputeODsThread();

    // while (!context_in_each_level_[level_].empty()) {
    //     ComputeODs(level_);
    //     if (IsTimeUp()) {
    //         break;
    //     }
    //     PruneLevels();
    //     CalculateNextLevel(level_);
    //     if (IsTimeUp()) {
    //         break;
    //     }

    //     level_++;
    // }
    
    // while (!context_in_each_level_[level_].empty()) {
    //     ComputeODs();
    //     if (IsTimeUp()) {
    //         break;
    //     }
    //     PruneLevels();
    //     CalculateNextLevel();
    //     if (IsTimeUp()) {
    //         break;
    //     }

    //     level_++;
    // }

    timer_.Stop();

    if (IsComplete()) {
        std::cout << "FastOD finished successfully" << '\n';
    } else {
        std::cout << "FastOD finished with a time-out" << '\n';
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

void Fastod::ComputeODs(std::size_t level) {
    const auto& context_this_level = context_in_each_level_[level];
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

        AddCandidates<false>(context, delAttrs, level);
        AddCandidates<true>(context, delAttrs, level);
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

void Fastod::PruneLevels(std::size_t level) {
    if (level >= 2) {
        auto& contexts = context_in_each_level_[level];

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

void Fastod::CalculateNextLevel(std::size_t current_level) {
    boost::unordered_map<AttributeSet, std::vector<size_t>> prefix_blocks;
    std::unordered_set<AttributeSet> context_next_level;

    const auto& context_this_level = context_in_each_level_[current_level];

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
