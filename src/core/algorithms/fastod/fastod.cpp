#include <iostream>
#include <utility>
#include <algorithm>

#include "fastod.h"
#include "single_attribute_predicate.h"
#include "stripped_partition.h"

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/thread.hpp>

namespace algos::fastod {

template <bool mutithread>
bool Fastod<mutithread>::IsTimeUp() const noexcept {
    return timer_.GetElapsedSeconds() >= time_limit_;
}

template <bool mutithread>
void Fastod<mutithread>::CCPut(size_t key, size_t attribute_set) noexcept {
    if constexpr(mutithread) {
        std::unique_lock lock(m_cc_);
        cc_[key] = attribute_set;
    } else {
        cc_[key] = attribute_set;
    }
}

template <bool mutithread>
size_t Fastod<mutithread>::CCGet(size_t key) noexcept {
    if constexpr(mutithread) {
        std::shared_lock lock(m_cc_);
        const auto it = cc_.find(key);
        if (it != cc_.cend())
            return it->second;
        return cc_[key] = 0;
    } else {
        const auto it = cc_.find(key);
        if (it != cc_.cend())
            return it->second;
        return cc_[key] = 0;
    }
}

template <bool mutithread>
void Fastod<mutithread>::addToRes(CanonicalOD<mutithread>&& od) {
    if constexpr(mutithread) {
        std::lock_guard lock(m_result_);
        result_.emplace_back(std::move(od));
    } else {
        result_.emplace_back(std::move(od));
    }
}

template <bool mutithread>
void Fastod<mutithread>::CSPut(size_t key, const AttributePair& value) noexcept {
    if constexpr(mutithread) {
        std::unique_lock lock(m_cs_);
        cs_[key].emplace(value);
    } else {
        cs_[key].emplace(value);
    }
}

template <bool mutithread>
void Fastod<mutithread>::CSPut(size_t key, AttributePair&& value) noexcept {
    if constexpr(mutithread) {
        std::unique_lock lock(m_cs_);
        cs_[key].emplace(std::move(value));
    } else {
        cs_[key].emplace(std::move(value));
    }
}

template <bool mutithread>
std::unordered_set<AttributePair>& Fastod<mutithread>::CSGet(size_t key) noexcept {
    if constexpr(mutithread) {
        std::shared_lock lock(m_cs_);
        return cs_[key];
    } else {
        return cs_[key];
    }
}

template <bool mutithread>
void Fastod<mutithread>::PrintStatistics() const noexcept {
    std::string last_od = result_.size() > 0
        ? result_[result_.size() - 1].ToString()
        : std::string("");
    
    // std::cout << "Current time " << timer_.GetElapsedSeconds() << " sec, "
    //           << "found od " << fd_count_ + ocd_count_ << " ones, where "
    //           << "fd " << fd_count_ << " ones, "
    //           << "ocd " << ocd_count_ << " ones, "
    //           << "the last od is " << last_od << '\n';
    
    std::cout << "RESULT: Time=" << timer_.GetElapsedSeconds() << ", "
              << "OD=" << fd_count_ + ocd_count_ << ", "
              << "FD=" << fd_count_ << ", "
              << "OCD=" << ocd_count_ << '\n';
}

template <bool mutithread>
bool Fastod<mutithread>::IsComplete() const noexcept {
    return is_complete_;
}

template <bool mutithread>
void Fastod<mutithread>::Initialize() noexcept {
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

template <bool mutithread>
std::vector<CanonicalOD<mutithread>> Fastod<mutithread>::Discover() noexcept {
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

    // std::cout << "Seconds elapsed: " << timer_.GetElapsedSeconds() << '\n'
    //           << "ODs found: " << fd_count_ + ocd_count_ << '\n'
    //           << "FDs found: " << fd_count_ << '\n'
    //           << "OCDs found: " << ocd_count_ << '\n';
    PrintStatistics();
    return result_;
}

template <bool mutithread>
void Fastod<mutithread>::ComputeODs() noexcept {
    const auto& context_this_level = context_in_each_level_[level_];
    Timer timer(true);
    std::vector<std::vector<size_t>> deletedAttrs(context_this_level.size());
    size_t contextInd = 0;
    for (size_t context : context_this_level) {
        auto& delAttrs = deletedAttrs[contextInd++];
        delAttrs.reserve(data_.GetColumnCount());
        for (size_t col = 0; col < data_.GetColumnCount(); ++col)
            delAttrs.push_back(deleteAttribute(context, col));
        if (IsTimeUp()) {
            is_complete_ = false;
            return;
        }

        size_t context_cc = schema_;
        for (ASIterator attr = attrsBegin(context); attr != attrsEnd(context); ++attr) {
            context_cc = intersect(context_cc, CCGet(delAttrs[*attr]));
        }

        CCPut(context, context_cc);

        if (level_ == 2) {
            for (size_t i = 0; i < data_.GetColumnCount(); i++) {
                for (size_t j = 0; j < data_.GetColumnCount(); j++) {
                    if (i == j) {
                        continue;
                    }
                    size_t c = attributeSet({i, j});

                    CSPut(c, AttributePair(SingleAttributePredicate(i, false), j));
                    CSPut(c, AttributePair(SingleAttributePredicate(i, true), j));
                }
            }
        } else if (level_ > 2) {
            std::unordered_set<AttributePair> candidate_cs_pair_set;
            for (ASIterator attr = attrsBegin(context); attr != attrsEnd(context); ++attr) {
                const auto& tmp = CSGet(delAttrs[*attr]);
                candidate_cs_pair_set.insert(tmp.cbegin(), tmp.cend());
            }

            for (AttributePair const& attribute_pair : candidate_cs_pair_set) {
                size_t context_delete_ab = deleteAttribute(
                    delAttrs[attribute_pair.left.attribute],
                    attribute_pair.right);

                bool add_context = true;
                for (ASIterator attr = attrsBegin(context_delete_ab);
                    attr != attrsEnd(context_delete_ab); ++attr) {
                    if (CSGet(delAttrs[*attr]).count(attribute_pair) == 0) {
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

    auto task = [this, &deletedAttrs](const std::tuple<size_t, size_t, std::unordered_set<size_t>::const_iterator,
            std::unordered_set<size_t>::const_iterator>& contexts) {
        auto [startPos, endPos, startIt, endIt] = contexts;
        auto getIsValid = [this](const CanonicalOD<mutithread>& od, size_t attrL, size_t attrR) {
            model::TypeId typeIdL = data_.GetColTypeId(attrL);
            model::TypeId typeIdR = data_.GetColTypeId(attrR);
            if (typeIdL == +model::TypeId::kInt) {
                if (typeIdR == +model::TypeId::kInt)
                    return od.template IsValid<int, int>(data_, error_rate_threshold_);
                else if (typeIdR == +model::TypeId::kDouble)
                    return od.template IsValid<int, double>(data_, error_rate_threshold_);
                else
                    return od.template IsValid<int, std::string>(data_, error_rate_threshold_);
            } else if (typeIdL == +model::TypeId::kDouble) {
                if (typeIdR == +model::TypeId::kInt)
                    return od.template IsValid<double, int>(data_, error_rate_threshold_);
                else if (typeIdR == +model::TypeId::kDouble)
                    return od.template IsValid<double, double>(data_, error_rate_threshold_);
                else
                    return od.template IsValid<double, std::string>(data_, error_rate_threshold_);
            }
            if (typeIdR == +model::TypeId::kInt)
                return od.template IsValid<std::string, int>(data_, error_rate_threshold_);
            else if (typeIdR == +model::TypeId::kDouble)
                return od.template IsValid<std::string, double>(data_, error_rate_threshold_);
            else
                return od.template IsValid<std::string, std::string>(data_, error_rate_threshold_);
        };
        for (size_t condInd = startPos; condInd < endPos; ++condInd, ++startIt) {
            size_t context = *startIt;
            const auto& delAttrs = deletedAttrs[condInd];
            if (IsTimeUp()) {
                is_complete_ = false;
                return;
            }

            size_t context_intersect_cc_context = intersect(context, CCGet(context));
            for (ASIterator attr = attrsBegin(context_intersect_cc_context);
                attr != attrsEnd(context_intersect_cc_context); ++attr) {
                CanonicalOD<mutithread> od(delAttrs[*attr], *attr);

                if (getIsValid(od, *attr, *attr)) {
                    addToRes(std::move(od));
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
                size_t a = it->left.attribute;
                size_t b = it->right;

                if (containsAttribute(CCGet(delAttrs[b]), a) &&
                    containsAttribute(CCGet(delAttrs[a]), b)) {
                    CanonicalOD<mutithread> od(deleteAttribute(delAttrs[a], b), it->left, b);
                    if (getIsValid(od, it->left.attribute, b)) {
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
    };
    if constexpr (mutithread) {
        std::vector<std::tuple<size_t, size_t, std::unordered_set<size_t>::const_iterator,
            std::unordered_set<size_t>::const_iterator>> contexts(threads_num_);
        size_t bucket_size = context_this_level.size() / threads_num_;
        size_t prevPos = 0;
        std::unordered_set<size_t>::const_iterator it = context_this_level.begin();
        size_t last_thread_work = context_this_level.size() % threads_num_;
        for (size_t i = 0; i < threads_num_; ++i) {
            auto it2 = it;
            if (i == threads_num_ - 1 && last_thread_work != 0)
                bucket_size = last_thread_work;
            for (size_t j = 0; j < bucket_size; ++j, ++it);
            contexts[i] = { prevPos, prevPos + bucket_size, it2, it };
            prevPos += bucket_size;
        }
        boost::asio::thread_pool pool(threads_num_);
        for (size_t i = 0; i < threads_num_; ++i) {
            boost::asio::post(pool, [&contexts, i, task]() { 
                task(contexts[i]);
            });
        }
        pool.join();
    } else {
        task({ (size_t)0, context_this_level.size(), context_this_level.begin(), context_this_level.end() });
    }
}

template <bool mutithread>
void Fastod<mutithread>::PruneLevels() noexcept {
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

template <bool mutithread>
void Fastod<mutithread>::CalculateNextLevel() noexcept {
    std::unordered_map<size_t, std::vector<size_t>> prefix_blocks;
    std::unordered_set<size_t> context_next_level;

    const auto& context_this_level = context_in_each_level_[level_];

    for (size_t attribute_set : context_this_level) {
        for (ASIterator attr = attrsBegin(attribute_set); 
            attr != attrsEnd(attribute_set); ++attr) {
            prefix_blocks[deleteAttribute(attribute_set, *attr)].push_back(*attr);
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

template class Fastod<false>;
template class Fastod<true>;
}
