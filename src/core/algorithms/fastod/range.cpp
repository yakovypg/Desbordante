#include <assert.h>
#include "range.h"

namespace algos::fastod {

Range::Range(std::size_t start) noexcept {
    start_ = end_ = start;
}

Range::Range(std::size_t start, std::size_t end) noexcept {
    assert(start <= end);  
    
    start_ = start;
    end_ = end;
}

std::size_t Range::GetStart() const noexcept {
    return start_;
}

std::size_t Range::GetEnd() const noexcept {
    return end_;
}

std::size_t Range::Size() const noexcept {
    return end_ - start_ + 1;
}

std::string Range::ToString() const {
    return "[" + std::to_string(start_) + ";"
               + std::to_string(end_) + "]";
}

std::vector<std::size_t> Range::ToVector() const {
    std::vector<std::size_t> values(Size());

    for (std::size_t i = start_; i <= end_; ++i) {
        values[i - start_] = i;
    }

    return values;
}

std::vector<Range> Range::ExtractRanges(std::vector<std::pair<int, size_t>> const& data,
                                        size_t start_index,
                                        size_t end_index) {
    std::vector<Range> ranges;

    size_t range_start = data[start_index].second;
    size_t prev_index = range_start;
    size_t curr_index;

    for (size_t i = start_index + 1; i <= end_index; ++i) {       
        curr_index = data[i].second;

        if (curr_index - prev_index != 1) {
            ranges.push_back(Range(range_start, prev_index));
            range_start = curr_index;
        }

        prev_index = curr_index;
    }

    ranges.push_back(Range(range_start, curr_index));

    return ranges;
}

} // namespace algos::fastod