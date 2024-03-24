#pragma once

#include <cstddef>

namespace algos::fastod::hashing {

inline size_t CombineHashes(size_t first, size_t second) {
    size_t wave = second + 2654435769UL + (first << 6) + (first >> 2);
    return first ^ wave;
}

}  // namespace algos::fastod::hashing