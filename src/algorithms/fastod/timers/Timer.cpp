#include <chrono>

#include "Timer.h"

using namespace algos::fastod;

Timer::Timer(bool start) noexcept { 
    if (start)
        Start();
}

void Timer::Start() noexcept {
    if (is_started_)
        return;
    
    start_time_ = end_time_ = std::chrono::system_clock::now();
    is_started_ = true;
}

void Timer::Stop() noexcept {
    if (!is_started_)
        return;
    
    end_time_ = std::chrono::system_clock::now();
    is_started_ = false;
}

bool Timer::IsStarted() const noexcept {
    return is_started_;
}

double Timer::GetElapsedSeconds() const {
    std::chrono::duration<double> elapsed_seconds = is_started_
        ? std::chrono::system_clock::now() - start_time_
        : end_time_ - start_time_;
    
    return elapsed_seconds.count();
}
