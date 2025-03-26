#include "pip_levelizer.hpp"


PipLevelizer::PipLevelizer(double start_price, double end_price, double percent) {
    this->start_price = start_price;
    this->end_price = end_price;
    this->percent = percent;
    this->levels = {};
    double current_price = start_price;
    while (current_price < end_price) {
        this->levels.push_back(current_price);
        current_price *= (1 + percent);
    }
    this->end_price = current_price;
}

std::ostream& operator<<(std::ostream& os, const PipLevelizer& pip_levelizer) {
    os << "PipLevelizer -> Start: " << pip_levelizer.start_price << " -  End: " << pip_levelizer.end_price << " -  Percent: " << pip_levelizer.percent << " -  Levels Count: " << pip_levelizer.levels.size() << std::endl;
    return os;
}

size_t PipLevelizer::get_level(double price) {
    for (size_t i = this->levels.size() - 1; i >= 0; i--) {
        if (price >= this->levels[i]) return i;
    }
    return this->levels.size();
}

size_t PipLevelizer::get_level_binary_search(double price) {
    size_t left = 0;
    size_t right = this->levels.size() - 1;
    size_t middle;
    while (left <= right) {
        middle = left + (right - left) / 2;
        if (price < this->levels[middle]) {
            right = middle - 1;
        } else if (price > this->levels[middle]) {
            left = middle + 1;
        } else {
            return middle;
        }
    }
    return right;
}
