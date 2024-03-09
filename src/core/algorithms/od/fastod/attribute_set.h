#pragma once

#include <bitset>
#include <cassert>
#include <climits>
#include <cstddef>
#include <functional>
#include <string>

#include <boost/functional/hash.hpp>

namespace algos::fastod {

class AttributeSet {
public:
    using size_type = size_t;

private:
    static constexpr size_type kBitsNum = 64;

    std::bitset<kBitsNum> bitset_;

    explicit AttributeSet(std::bitset<kBitsNum> bitset) noexcept : bitset_(std::move(bitset)) {}

public:
    AttributeSet() noexcept = default;

    explicit AttributeSet([[maybe_unused]] size_type attrs) noexcept {
        assert(attrs < kBitsNum);
    }

    explicit AttributeSet([[maybe_unused]] size_type attrs, size_type value) noexcept
        : bitset_(value) {
        assert(attrs < kBitsNum);
    }

    AttributeSet& operator&=(AttributeSet const& b) noexcept {
        bitset_ &= b.bitset_;
        return *this;
    }

    AttributeSet& operator|=(AttributeSet const& b) noexcept {
        bitset_ |= b.bitset_;
        return *this;
    }

    AttributeSet& operator^=(AttributeSet const& b) noexcept {
        bitset_ ^= b.bitset_;
        return *this;
    }

    AttributeSet& set(size_type n, bool val = true) {
        bitset_.set(n, val);
        return *this;
    }

    AttributeSet& reset(size_type n) {
        bitset_.reset(n);
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

    size_type size() const noexcept {
        return bitset_.size();
    }

    size_type find_first() const noexcept {
        return bitset_._Find_first();
    }

    size_type find_next(size_type pos) const noexcept {
        return bitset_._Find_next(pos);
    }

    friend AttributeSet operator&(AttributeSet const& b1, AttributeSet const& b2) noexcept;
    friend AttributeSet operator|(AttributeSet const& b1, AttributeSet const& b2) noexcept;
    friend AttributeSet operator^(AttributeSet const& b1, AttributeSet const& b2) noexcept;
    friend bool operator==(AttributeSet const& b1, AttributeSet const& b2) noexcept;
    friend bool operator!=(AttributeSet const& b1, AttributeSet const& b2) noexcept;
    friend struct std::hash<AttributeSet>;
    friend struct boost::hash<AttributeSet>;
};

inline AttributeSet operator&(AttributeSet const& b1, AttributeSet const& b2) noexcept {
    AttributeSet as(b1.bitset_ & b2.bitset_);
    return as;
}

inline AttributeSet operator|(AttributeSet const& b1, AttributeSet const& b2) noexcept {
    AttributeSet as(b1.bitset_ | b2.bitset_);
    return as;
}

inline AttributeSet operator^(AttributeSet const& b1, AttributeSet const& b2) noexcept {
    AttributeSet as(b1.bitset_ ^ b2.bitset_);
    return as;
}

inline bool operator==(AttributeSet const& b1, AttributeSet const& b2) noexcept {
    return b1.bitset_ == b2.bitset_;
}

inline bool operator!=(AttributeSet const& b1, AttributeSet const& b2) noexcept {
    return !(b1 == b2);
}

}  // namespace algos::fastod

template <>
struct std::hash<algos::fastod::AttributeSet> {
    size_t operator()(algos::fastod::AttributeSet const& x) const noexcept {
        return x.bitset_.to_ullong();
    }
};

template <>
struct boost::hash<algos::fastod::AttributeSet> {
    size_t operator()(algos::fastod::AttributeSet const& x) const noexcept {
        return x.bitset_.to_ullong();
    }
};

namespace algos::fastod {

inline AttributeSet attributeSet(std::initializer_list<AttributeSet::size_type> attributes,
                                 AttributeSet::size_type size) {
    AttributeSet attr_set(size);
    for (auto const attr : attributes) {
        attr_set.set(attr);
    }
    return attr_set;
}

inline bool containsAttribute(AttributeSet const& value,
                              AttributeSet::size_type attribute) noexcept {
    return value.test(attribute);
}

inline AttributeSet addAttribute(AttributeSet const& value, AttributeSet::size_type attribute) {
    auto value_copy = value;
    return value_copy.set(attribute);
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

inline AttributeSet::size_type getAttributeCount(AttributeSet const& value) noexcept {
    return value.count();
}

}  // namespace algos::fastod
