#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <optional>

namespace algos::fastod {

class Range {
public:
    Range() noexcept = default;
    Range(Range const& other) = default;
    Range(std::size_t start, std::size_t end) noexcept;
    explicit Range(std::size_t start) noexcept;
    
    std::size_t GetStart() const noexcept;
    std::size_t GetEnd() const noexcept;
    std::size_t Size() const noexcept;

    std::string ToString() const;
    std::vector<std::size_t> ToVector() const;

    Range& operator=(Range const& other);
    bool operator<(Range const& other) const;
    bool operator>(Range const& other) const;
    bool operator==(Range const& other) const;
    bool operator!=(Range const& other) const;

    static std::optional<Range> Intersect(Range const& lhs, Range const& rhs);
    static std::vector<Range> Differense(Range const& lhs, Range const& rhs);
    static std::optional<Range> DifferenseWithLeftShift(Range const& lhs, Range const& rhs);

    static std::vector<Range> ExtractRanges(std::vector<std::pair<int, size_t>> const& data);
    static std::vector<Range> ExtractRanges(std::vector<std::pair<int, size_t>> const& data,
                                            size_t start_index,
                                            size_t end_index);

private:
    std::size_t start_;
    std::size_t end_;
};

} // namespace algos::fastod