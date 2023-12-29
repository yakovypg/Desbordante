#pragma once

#include <unordered_map>
#include <queue>
#include <cstddef>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>

namespace algos::fastod {

template <typename K, typename V>
class CacheWithLimit {
private:
    std::unordered_map<K, V> entries_;
    std::queue<K> keys_in_order_;
    const std::size_t max_size_;

public:
    explicit CacheWithLimit(std::size_t max_size) : max_size_(max_size) {};
    
    bool Contains(const K& key) const noexcept {
        return entries_.count(key) != 0;
    }
    const V& Get(const K& key) const {
        return entries_.at(key);
    }
    void Set(const K& key, const V& value) {
        if (keys_in_order_.size() >= max_size_) {
            entries_.erase(keys_in_order_.front());
            keys_in_order_.pop();
        }

        keys_in_order_.push(key);
        entries_.emplace(key, value);
    }
};

} // namespace algos::fastod;
