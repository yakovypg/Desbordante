#pragma once

#include <string>
#include <vector>
#include <cstddef>

namespace algos::fastod {

class Range {
public:
    Range() noexcept = default;
    Range(std::size_t start, std::size_t end) noexcept;
    explicit Range(std::size_t start) noexcept;
    
    std::size_t GetStart() const noexcept;
    std::size_t GetEnd() const noexcept;
    std::size_t Size() const noexcept;

    std::string ToString() const;
    std::vector<std::size_t> ToVector() const;

    static std::vector<Range> ExtractRanges(std::vector<std::pair<int, size_t>> const& data,
                                            size_t start_index,
                                            size_t end_index);

private:
    std::size_t start_;
    std::size_t end_;
};

} // namespace algos::fastod