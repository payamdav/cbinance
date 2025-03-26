#include "volume_profiler.hpp"

VolumeProfiler::VolumeProfiler(PipLevelizer levelizer) : levelizer(levelizer), volumes(levelizer.levels.size(), 0), min_level(levelizer.levels.size() - 1), max_level(0) {}

void VolumeProfiler::push(size_t l, double volume) {
    if (l < min_level) min_level = l;
    if (l > max_level) max_level = l;
    volumes[l] += volume;
}

size_t VolumeProfiler::size() {
    return max_level - min_level + 1;
}

vector<double> VolumeProfiler::get_volumes() {
    vector<double> result;
    for (size_t i = min_level; i <= max_level; i++) {
        result.push_back(volumes[i]);
    }
    return result;
}

vector<double> VolumeProfiler::get_levels() {
    vector<double> result;
    for (size_t i = min_level; i <= max_level; i++) {
        result.push_back(levelizer.levels[i]);
    }
    return result;
}

