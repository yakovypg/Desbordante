#pragma once

#include <chrono>

namespace algos::fastod {

typedef std::chrono::_V2::high_resolution_clock::time_point TimePoint;

class Timer {
private:
    bool is_started_;
    TimePoint start_time_;
    TimePoint end_time_;

public:
    explicit Timer(bool start = false);

    void Start();
    void Stop();

    bool IsStarted() const;
    double GetElapsedSeconds() const;
};

} // namespace algos::fastod
