#pragma once

#include <bitset>

#include <bitset>
#include <cassert>
#include <cstddef>
#include <string>
#include <functional>

namespace algos::fastod {

#if 0
using AttributeSet = boost::dynamic_bitset<size_t>;
#endif

class AttributeSet {
private:
    static constexpr size_t kBitsNum = 64;

    std::bitset<kBitsNum> bitset_;

    explicit AttributeSet(std::bitset<kBitsNum> bitset) noexcept
        : bitset_(std::move(bitset)) {}

public:
    using size_type = size_t;

    constexpr static size_type npos = static_cast<size_type>(-1);

    AttributeSet() noexcept = default;
    explicit AttributeSet([[maybe_unused]] size_type attrs) noexcept {
        assert(attrs < kBitsNum);
    }
    explicit AttributeSet([[maybe_unused]] size_type attrs, size_type value) noexcept
        : bitset_(value) {
        assert(attrs < kBitsNum);
    }

    AttributeSet& operator&=(const AttributeSet& b) noexcept {
        bitset_ &= b.bitset_;
        return *this;
    }
    AttributeSet& operator|=(const AttributeSet& b) noexcept {
        bitset_ |= b.bitset_;
        return *this;
    }
    AttributeSet& operator^=(const AttributeSet& b) noexcept {
        bitset_ ^= b.bitset_;
        return *this;
    }

    AttributeSet& set(size_type n, bool val = true) {
        bitset_.set(n, val);
        return *this;
    }
    AttributeSet& set() noexcept {
        bitset_.set();
        return *this;
    }
    AttributeSet& reset(size_type n) {
        bitset_.reset(n);
        return *this;
    }
    AttributeSet& reset() noexcept {
        bitset_.reset();
        return *this;
    }
    bool test(size_type n) const noexcept {
        return bitset_.test(n);
    }
    bool all() const noexcept {
        return bitset_.all();
    }
    bool any() const noexcept {
        return bitset_.any();
    }
    bool none() const noexcept {
        return bitset_.none();
    }
    AttributeSet operator~() const noexcept {
        AttributeSet as(~bitset_);
        return as;
    }
    size_type count() const noexcept {
        return bitset_.count();
    }

    size_type find_first() const noexcept {
        if constexpr (sizeof(unsigned long long) <= kBitsNum) {
            // relying on the fact that npos == -1
            return __builtin_ffsll(bitset_.to_ullong()) - 1;
        } else {
            for (size_type i = 0; i != bitset_.size(); ++i) {
                if (bitset_[i] == 1) {
                    return i;
                }
            }
            return npos;
        }
    }
    size_type find_next(size_type pos) const noexcept {
        if constexpr (sizeof(unsigned long long) <= kBitsNum) {
            unsigned long long mask = ~((1 << (pos + 1)) - 1);
            unsigned long long value = bitset_.to_ullong() & mask;
            return __builtin_ffsll(value) - 1;
        } else {
            for (size_type i = pos + 1; i != bitset_.size(); ++i) {
                if (bitset_[i] == 1) {
                    return i;
                }
            }
            return npos;
        }
    }

    friend AttributeSet operator&(const AttributeSet& b1, const AttributeSet& b2) noexcept;
    friend AttributeSet operator|(const AttributeSet& b1, const AttributeSet& b2) noexcept;
    friend AttributeSet operator^(const AttributeSet& b1, const AttributeSet& b2) noexcept;
    friend bool operator==(const AttributeSet& b1, const AttributeSet& b2) noexcept;
    friend bool operator!=(const AttributeSet& b1, const AttributeSet& b2) noexcept;
    friend struct std::hash<AttributeSet>;
};

inline AttributeSet operator&(const AttributeSet& b1, const AttributeSet& b2) noexcept {
    AttributeSet as(b1.bitset_ & b2.bitset_);
    return as;
}

inline AttributeSet operator|(const AttributeSet& b1, const AttributeSet& b2) noexcept {
    AttributeSet as(b1.bitset_ | b2.bitset_);
    return as;
}

inline AttributeSet operator^(const AttributeSet& b1, const AttributeSet& b2) noexcept {
    AttributeSet as(b1.bitset_ ^ b2.bitset_);
    return as;
}

inline bool operator==(const AttributeSet& b1, const AttributeSet& b2) noexcept {
    return b1.bitset_ == b2.bitset_;
}

inline bool operator!=(const AttributeSet& b1, const AttributeSet& b2) noexcept {
    return !(b1 == b2);
}

}

template <>
struct std::hash<algos::fastod::AttributeSet>
{
    size_t operator()(algos::fastod::AttributeSet const& x) const noexcept {
        return std::hash<std::bitset<algos::fastod::AttributeSet::kBitsNum>>()(x.bitset_);
    }
};


namespace algos::fastod {

inline AttributeSet attributeSet(std::initializer_list<size_t> attributes, size_t size) {
    AttributeSet attr_set(size);
    for (auto attr : attributes) {
        attr_set.set(attr);
    }
    return attr_set;
}

inline bool containsAttribute(AttributeSet const& value, AttributeSet::size_type attribute) noexcept {
    return value.test(attribute);
}

inline AttributeSet addAttribute(AttributeSet const& value, AttributeSet::size_type attribute) {
    auto value_copy = value;
    return value_copy.set(attribute);
}

inline void addAttributeTo(AttributeSet& value, AttributeSet::size_type attribute) {
    value.set(attribute);
}

inline AttributeSet deleteAttribute(AttributeSet const& value, AttributeSet::size_type attribute) {
    auto value_copy = value;
    return value_copy.reset(attribute);
}

inline AttributeSet intersect(AttributeSet const& value1, AttributeSet const& value2) noexcept {
    return value1 & value2;
}

inline AttributeSet difference(AttributeSet const& value1, AttributeSet const& value2) noexcept {
    return value1 & (~value2);
}

inline bool isEmptyAS(AttributeSet const& value) noexcept {
    return value.none();
}

std::string ASToString(AttributeSet const& value);
inline std::size_t getAttributeCount(AttributeSet const& value) noexcept {
    return value.count();
}

} // namespace algos::fastod
