#include "timer.hpp"

Timer::Timer() : start(std::chrono::high_resolution_clock::now()) {}

void Timer::reset() {
    start = std::chrono::high_resolution_clock::now();
}

double Timer::elapsed() const {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<double>(duration.count()) / 1000.0;
}
