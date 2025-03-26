#pragma once
#include <vector>
#include "../pip_levelizer/pip_levelizer.hpp"

using namespace std;


class VolumeProfiler {
    public:
        vector<double> volumes;
        PipLevelizer levelizer;
        size_t min_level;
        size_t max_level;

        VolumeProfiler(PipLevelizer levelizer);
        void push(size_t l, double volume);
        size_t size();
        vector<double> get_volumes();
        vector<double> get_levels();
};
