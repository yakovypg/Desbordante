#include <assert.h>
#include <optional>

#include "range.h"

namespace algos::fastod {

Range::Range(std::size_t start) noexcept : start_(start), end_(start) {}

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

Range& Range::operator=(Range const& other) {
    if (this == &other) {
        return *this;
    }

    start_ = other.start_;
    end_ = other.end_;

    return *this;
}

bool Range::operator<(Range const& other) const {
    return end_ < other.start_;
}

bool Range::operator>(Range const& other) const {
    return start_ > other.end_;
}

bool Range::operator==(Range const& other) const {
    return start_ == other.start_ && end_ == other.end_;
}

bool Range::operator!=(Range const& other) const {
    return !(*this == other);
}

std::optional<Range> Range::Intersect(Range const& lhs, Range const& rhs) {
    size_t start = std::max(lhs.start_, rhs.start_);
    size_t end = std::min(lhs.end_, rhs.end_);

    return start <= end
        ? std::optional<Range>{ Range(start, end) }
        : std::nullopt;
}

std::vector<Range> Range::Differense(Range const& lhs, Range const& rhs) {
    std::optional<Range> intersection = Intersect(lhs, rhs);

    if (!intersection.has_value()) {
        return { lhs };
    }

    std::vector<Range> diff;
    Range range = intersection.value();

    if (lhs.start_ < range.start_) {
        diff.push_back(Range(lhs.start_, range.start_ - 1));
    }

    if (range.end_ < lhs.end_) {
        diff.push_back(Range(range.end_ + 1, lhs.end_));
    }

    return diff;
}

// [1-10] & [2-5] = [6-10]
std::optional<Range> Range::DifferenseWithLeftShift(Range const& lhs, Range const& rhs) {
    std::optional<Range> intersection = Intersect(lhs, rhs);

    if (!intersection.has_value()) {
        return std::optional<Range> { lhs };
    }

    size_t start = intersection.value().GetEnd() + 1;
    
    return start <= lhs.end_
        ? std::optional<Range>{ Range(start, lhs.end_) }
        : std::nullopt;
}

std::vector<Range> Range::ExtractRanges(std::vector<std::pair<int, size_t>> const& data) {
    return ExtractRanges(data, 0, data.size() - 1);
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