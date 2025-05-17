#pragma once
#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

class Timer {
public:
    Timer();

    void reset();
    double elapsed() const;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

#endif // TIMER_HPP
