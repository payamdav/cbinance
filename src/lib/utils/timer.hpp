#pragma once
#include <chrono>
#include <iostream>


namespace utils {

class Timer {
    public:
    std::chrono::_V2::system_clock::time_point start, last;
    Timer() {
        start = std::chrono::high_resolution_clock::now();
        last = start;
    }

    void reset() {
        start = std::chrono::high_resolution_clock::now();
        last = start;
    }

    void checkpoint() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last);
        auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);
        std::cout << "Duration: " << duration.count() << "s Laps  --  " << total_duration.count() << "s From Start." << std::endl;
        last = now;
    }

};

}
