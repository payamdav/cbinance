#include "pip_levelizer.hpp"
#include <iostream>


using namespace std;


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

size_t PipLevelizer::get_level(const double & price) {
    for (size_t i = this->levels.size() - 1; i >= 0; i--) {
        if (price >= this->levels[i]) {
            return i;
        }
    }
    return 0;
}

size_t PipLevelizer::get_level_binary_search(const double & price) {
    size_t left = 0;
    size_t right = this->levels.size() - 1;
    // Handle edge cases
    if (price < this->levels[left]) {
        return left; // Return the first level if price is below the first level
    }
    if (price >= this->levels[right]) {
        return right; // Return the last level if price is above or equal to the last level
    }
    // Binary search for the level
    while (left < right - 1) {
        size_t mid = left + (right - left) / 2; // Prevents overflow
        if (this->levels[mid] > price) {
            right = mid - 1; // Move the right pointer to mid - 1 if mid level is greater than price
        } else {
            left = mid; // Move the left pointer to mid if mid level is less than or equal to price
        }
    }
    for (size_t i = right; i >= left; i--) { // Check the last two levels to ensure we find the correct level
        if (this->levels[i] <= price) {
            return i; // Update left to the highest level that is less than or equal to price
        }
    }
}

size_t PipLevelizer::operator () (const double & price) {
    return this->get_level(price);
}

bool PipLevelizer::is_in_range(const double & price) {
    return (price >= this->start_price && price < this->end_price);
}
